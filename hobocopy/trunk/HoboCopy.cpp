/* 
Copyright (c) 2006 Wangdera Corporation (hobocopy@wangdera.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


#include "stdafx.h"
#include "CComException.h"
#include "CHoboCopyException.h"
#include "COptions.h"
#include "CRecursiveAction.h"
#include "CCopyRecursive.h"
#include "InstrumentationHelper.h"

bool s_cancel = false; 


// Forward declarations
void CalculateSourcePath(LPCTSTR wszSnapshotDevice, LPCTSTR wszBackupSource, LPCTSTR wszMountPoint, CString& output);
void Cleanup(bool bAbnormalAbort, bool bSnapshotcreated, CComPtr<IVssBackupComponents> pBackupComponents, GUID snapshotSetId);
bool Confirm(LPCTSTR message); 
void CopyRecursive(LPCTSTR wszSource, LPCTSTR wszDestination, bool skipDenied, CCopyFilter& filter);
BOOL WINAPI CtrlHandler(DWORD dwCtrlType); 
void DeleteRecursive(LPCTSTR target); 
void RecurseDirectory(LPCTSTR srcbase, CRecursiveAction& action, LPCTSTR directory);
LPCSTR WideToNarrow(LPCWSTR wsz);

int _tmain(int argc, _TCHAR* argv[])
{
    //::DebugBreak(); 

    InstrumentationHelper::SetLogLevel(LOG_LEVEL_WARN); 
    Console::WriteLine(TEXT("HoboCopy (c) 2006 Wangdera Corporation. hobocopy@wangdera.com")); 
    Console::WriteLine(TEXT("")); 

    GUID snapshotSetId = GUID_NULL; 
    bool bSnapshotCreated = false;
    bool bAbnormalAbort = true; 
    CComPtr<IVssBackupComponents> pBackupComponents; 

    int fileCount = 0; 
    LONGLONG byteCount = 0; 
    int directoryCount = 0; 
    int skipCount = 0; 
    SYSTEMTIME startTime;
    try
    {
        COptions options = COptions::Parse(argc, argv); 

        if (options.get_Debug())
        {
            ::DebugBreak(); 
        }

        InstrumentationHelper::SetLogLevel((LOG_LEVEL) options.get_LogLevel()); 

        InstrumentationHelper::Log(TEXT("Calling CoInitialize")); 
        CHECK_HRESULT(::CoInitialize(NULL)); 
        CHECK_HRESULT(
            ::CoInitializeSecurity(
            NULL, 
            -1, 
            NULL, 
            NULL, 
            RPC_C_AUTHN_LEVEL_PKT_PRIVACY, 
            RPC_C_IMP_LEVEL_IDENTIFY, 
            NULL, 
            EOAC_NONE, 
            NULL)); 

        ::GetSystemTime(&startTime); 
        CString startTimeString; 
        Utilities::FormatDateTime(&startTime, TEXT(" "), false, startTimeString); 

        CString message; 
        message.AppendFormat(TEXT("Starting a %s backup from %s to %s"), 
            options.get_BackupType() == VSS_BT_FULL ? TEXT("full") : TEXT("incremental"), 
            options.get_Source(), 
            options.get_Destination()); 
        InstrumentationHelper::Log(message, LOG_LEVEL_INFO); 

        if (options.get_ClearDestination())
        {
            CString message; 
            message.AppendFormat(TEXT("Recursively deleting destination directory %s."), 
                options.get_Destination()); 
            InstrumentationHelper::Log(message, LOG_LEVEL_INFO); 

            bool doDelete = options.get_AcceptAll(); 

            if (!doDelete)
            {
                if (Confirm(message))
                {
                    doDelete = true; 
                }
                else
                {
                    InstrumentationHelper::Log(TEXT("Aborting backup."), LOG_LEVEL_INFO); 
                    return 3; 
                }
            }

            if (doDelete)
            {
                DeleteRecursive(options.get_Destination()); 
            }
        }

        CBackupState backupState; 

        LPSYSTEMTIME lastBackupTime; 

        if (options.get_BackupType() == VSS_BT_INCREMENTAL)
        {
            backupState.Load(options.get_StateFile()); 

            LPSYSTEMTIME lastFullBackupTime = backupState.get_LastFullBackupTime(); 
            LPSYSTEMTIME lastIncrementalBackupTime = backupState.get_LastIncrementalBackupTime(); 
            if (lastIncrementalBackupTime != NULL)
            {
                lastBackupTime = lastIncrementalBackupTime; 
            }
            else
            {
                lastBackupTime = lastFullBackupTime; 
            }
        }


        InstrumentationHelper::Log(TEXT("Calling CreateVssBackupComponents")); 
        CHECK_HRESULT(::CreateVssBackupComponents(&pBackupComponents)); 

        InstrumentationHelper::Log(TEXT("Calling InitializeForBackup")); 
        CHECK_HRESULT(pBackupComponents->InitializeForBackup()); 

        CComPtr<IVssAsync> pWriterMetadataStatus; 

        InstrumentationHelper::Log(TEXT("Calling GatherWriterMetadata")); 
        CHECK_HRESULT(pBackupComponents->GatherWriterMetadata(&pWriterMetadataStatus)); 

        InstrumentationHelper::Log(TEXT("Waiting for writer metadata")); 
        CHECK_HRESULT(pWriterMetadataStatus->Wait()); 

        HRESULT hrGatherStatus; 
        InstrumentationHelper::Log(TEXT("Calling QueryStatus for GatherWriterMetadata")); 
        CHECK_HRESULT(pWriterMetadataStatus->QueryStatus(&hrGatherStatus, NULL)); 

        if (hrGatherStatus == VSS_S_ASYNC_CANCELLED)
        {
            throw new CHoboCopyException(L"GatherWriterMetadata was cancelled."); 
        }

        InstrumentationHelper::Log(TEXT("Calling GetWriterMetadataCount")); 
        UINT cWriters; 
        CHECK_HRESULT(pBackupComponents->GetWriterMetadataCount(&cWriters)); 

        for (UINT i = 0; i < cWriters; ++i)
        {
            CComPtr<IVssExamineWriterMetadata> pExamineWriterMetadata; 
            GUID id; 
            InstrumentationHelper::Log(TEXT("Calling GetWriterMetadata")); 
            CHECK_HRESULT(pBackupComponents->GetWriterMetadata(i, &id, &pExamineWriterMetadata)); 
            GUID idInstance; 
            GUID idWriter; 
            BSTR bstrWriterName;
            VSS_USAGE_TYPE usage; 
            VSS_SOURCE_TYPE source; 
            CHECK_HRESULT(pExamineWriterMetadata->GetIdentity(&idInstance, &idWriter, &bstrWriterName, &usage, &source)); 

            CComBSTR writerName(bstrWriterName); 
            CString message; 
            message.AppendFormat(TEXT("Writer %d named %s"), i, (LPCTSTR) writerName); 
            InstrumentationHelper::Log(message); 
        }

        InstrumentationHelper::Log(TEXT("Calling StartSnapshotSet")); 
        CHECK_HRESULT(pBackupComponents->StartSnapshotSet(&snapshotSetId));

        InstrumentationHelper::Log(TEXT("Calling GetVolumePathName")); 
        WCHAR wszVolumePathName[MAX_PATH]; 
        BOOL bWorked = ::GetVolumePathName(options.get_Source(), wszVolumePathName, MAX_PATH); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError(); 
            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("There was an error retrieving the volume name from the path. Path: %s Error: %s"), 
                options.get_Source(), errorMessage); 
            throw new CHoboCopyException(message.GetString()); 
        }

        InstrumentationHelper::Log(TEXT("Calling AddToSnapshotSet")); 
        GUID snapshotId; 
        CHECK_HRESULT(pBackupComponents->AddToSnapshotSet(wszVolumePathName, GUID_NULL, &snapshotId)); 

        InstrumentationHelper::Log(TEXT("Calling SetBackupState")); 
        CHECK_HRESULT(pBackupComponents->SetBackupState(FALSE, FALSE, options.get_BackupType(), FALSE)); 

        InstrumentationHelper::Log(TEXT("Calling PrepareForBackup")); 
        CComPtr<IVssAsync> pPrepareForBackupResults; 
        CHECK_HRESULT(pBackupComponents->PrepareForBackup(&pPrepareForBackupResults)); 

        CHECK_HRESULT(pPrepareForBackupResults->Wait()); 

        HRESULT hrPrepareForBackupResults; 
        CHECK_HRESULT(pPrepareForBackupResults->QueryStatus(&hrPrepareForBackupResults, NULL)); 

        if (hrPrepareForBackupResults != VSS_S_ASYNC_FINISHED)
        {
            throw new CHoboCopyException(TEXT("Prepare for backup failed.")); 
        }

        SYSTEMTIME snapshotTime; 
        ::GetSystemTime(&snapshotTime); 

        bWorked = ::SetConsoleCtrlHandler(CtrlHandler, TRUE); 

        if (!bWorked)
        {
            InstrumentationHelper::Log(TEXT("Unable to set control handler. Ctrl-C and Ctrl-Break may have undesirable results."), LOG_LEVEL_WARN);
        }

        InstrumentationHelper::Log(TEXT("Calling DoSnapshotSet")); 
        CComPtr<IVssAsync> pDoSnapshotSetResults;
        CHECK_HRESULT(pBackupComponents->DoSnapshotSet(&pDoSnapshotSetResults)); 

        CHECK_HRESULT(pDoSnapshotSetResults->Wait());

        bSnapshotCreated = true; 

        if (s_cancel)
        {
            throw new CHoboCopyException(TEXT("Processing was cancelled by control-c, control-break, or a shutdown event. Terminating.")); 
        }

        bWorked = ::SetConsoleCtrlHandler(CtrlHandler, FALSE); 

        if (!bWorked)
        {
            InstrumentationHelper::Log(TEXT("Unable to reset control handler. Ctrl-C and Ctrl-Break may have undesirable results."), LOG_LEVEL_WARN);
        }

        HRESULT hrDoSnapshotSetResults; 
        CHECK_HRESULT(pDoSnapshotSetResults->QueryStatus(&hrDoSnapshotSetResults, NULL)); 

        if (hrDoSnapshotSetResults != VSS_S_ASYNC_FINISHED)
        {
            throw new CHoboCopyException(L"DoSnapshotSet failed."); 
        }

        InstrumentationHelper::Log(TEXT("Calling GetSnapshotProperties")); 
        VSS_SNAPSHOT_PROP snapshotProperties; 
        CHECK_HRESULT(pBackupComponents->GetSnapshotProperties(snapshotId, &snapshotProperties));

        InstrumentationHelper::Log(TEXT("Calling CalculateSourcePath")); 
        // TODO: We'll eventually have to deal with mount points
        CString wszSource;
        CalculateSourcePath(
            snapshotProperties.m_pwszSnapshotDeviceObject, 
            options.get_Source(),
            wszVolumePathName, 
            wszSource
            );

        InstrumentationHelper::Log(TEXT("Calling CopyRecursive")); 

        CCopyFilter* pFilter; 
        if (options.get_BackupType() == VSS_BT_FULL)
        {
            pFilter = new CIncludeAllCopyFilter(); 
        }
        else if (options.get_BackupType() == VSS_BT_INCREMENTAL)
        {
            pFilter = new CModifiedSinceCopyFilter(lastBackupTime); 
        }

        CCopyRecursive copyAction(wszSource, options.get_Destination(), options.get_SkipDenied(), *pFilter); 
        RecurseDirectory(wszSource, copyAction, TEXT("")); 

        delete pFilter; 

        fileCount = copyAction.get_FileCount(); 
        directoryCount = copyAction.get_DirectoryCount();
        skipCount = copyAction.get_SkipCount(); 
        byteCount = copyAction.get_ByteCount(); 

        InstrumentationHelper::Log(TEXT("Calling BackupComplete")); 
        CComPtr<IVssAsync> pBackupCompleteResults; 
        CHECK_HRESULT(pBackupComponents->BackupComplete(&pBackupCompleteResults)); 

        HRESULT hrBackupCompleteResults; 
        CHECK_HRESULT(pBackupCompleteResults->QueryStatus(&hrBackupCompleteResults, NULL)); 

        if (hrPrepareForBackupResults != VSS_S_ASYNC_FINISHED)
        {
            throw new CHoboCopyException(TEXT("Completion of backup failed.")); 
        }

        bAbnormalAbort = false; 

        if (options.get_StateFile() != NULL)
        {
            InstrumentationHelper::Log(TEXT("Calling SaveAsXML"));
            CComBSTR bstrBackupDocument; 
            CHECK_HRESULT(pBackupComponents->SaveAsXML(&bstrBackupDocument)); 

            if (options.get_BackupType() == VSS_BT_FULL)
            {
                backupState.set_LastFullBackupTime(&snapshotTime); 
            }
            else if (options.get_BackupType() == VSS_BT_INCREMENTAL)
            {
                backupState.set_LastIncrementalBackupTime(&snapshotTime); 
            }
            else
            {
                throw new CHoboCopyException(TEXT("Unsupported backup type.")); 
            }

            backupState.Save(options.get_StateFile(), bstrBackupDocument); 
        }
    }
    catch (CComException* e)
    {
        Cleanup(bAbnormalAbort, bSnapshotCreated, pBackupComponents, snapshotSetId);
        CString message; 
        CString file; 
        e->get_File(file); 
        message.Format(TEXT("There was a COM failure 0x%x - %s (%d)"), 
            e->get_Hresult(), file, e->get_Line()); 
        InstrumentationHelper::Log(message, LOG_LEVEL_ERROR); 
        return 1; 
    }
    catch (CHoboCopyException* e)
    {
        Cleanup(bAbnormalAbort, bSnapshotCreated, pBackupComponents, snapshotSetId);
        InstrumentationHelper::Log(e->get_Message(), LOG_LEVEL_ERROR); 
        return 1; 
    }
    catch (CParseOptionsException* e)
    {
        Cleanup(bAbnormalAbort, bSnapshotCreated, pBackupComponents, snapshotSetId);
        CString message; 
        message.AppendFormat(TEXT("Error: %s\n"), e->get_Message()); 
        InstrumentationHelper::Log(message, LOG_LEVEL_ERROR);
        InstrumentationHelper::Log(COptions::get_Usage(), LOG_LEVEL_ERROR); 
        return 2; 
    }

    Cleanup(false, bSnapshotCreated, pBackupComponents, snapshotSetId);
    Console::WriteLine(TEXT("Backup successfully completed.")); 
    
    CString message; 
    CString startTimeStringLocal; 
    Utilities::FormatDateTime(&startTime, TEXT(" "), true, startTimeStringLocal); 
    CString finishTimeString; 
    SYSTEMTIME finishTime; 
    ::GetSystemTime(&finishTime); 
    Utilities::FormatDateTime(&finishTime, TEXT(" "), true, finishTimeString); 
    message.AppendFormat(TEXT("Backup started at %s, completed at %s."), 
        startTimeStringLocal, finishTimeString); 
    InstrumentationHelper::Log(message, LOG_LEVEL_INFO); 
    message.Empty(); 

    float unitCount = byteCount; 
    LPCTSTR units = TEXT("bytes"); 

    if (unitCount > 1024)
    {
        unitCount = unitCount / 1024.0F; 
        units = TEXT("KB"); 
    }

    if (unitCount > 1024)
    {
        unitCount = unitCount / 1024.0F; 
        units = TEXT("MB"); 
    }

    if (unitCount > 1024)
    {
        unitCount = unitCount / 1024.0F; 
        units = TEXT("GB"); 
    }

    message.AppendFormat(TEXT("%d files (%.2f %s, %d directories) copied, %d files skipped"), 
        fileCount, unitCount, units, directoryCount, skipCount); 
    InstrumentationHelper::Log(message, LOG_LEVEL_INFO); 

    return 0;
}


void CalculateSourcePath(LPCTSTR wszSnapshotDevice, LPCTSTR wszBackupSource, LPCTSTR wszMountPoint, CString& output)
{
    CString backupSource(wszBackupSource); 
    CString mountPoint(wszMountPoint); 

    CString subdirectory = backupSource.Mid(mountPoint.GetLength()); 

    Utilities::CombinePath(wszSnapshotDevice, subdirectory, output); 
}

void Cleanup(bool bAbnormalAbort, bool bSnapshotCreated, CComPtr<IVssBackupComponents> pBackupComponents, GUID snapshotSetId)
{
    if (pBackupComponents == NULL)
    {
        return; 
    }

    if (bAbnormalAbort)
    {
        pBackupComponents->AbortBackup(); 
    }
    if (bSnapshotCreated)
    {
        LONG cDeletedSnapshots; 
        GUID nonDeletedSnapshotId; 
        pBackupComponents->DeleteSnapshots(snapshotSetId, VSS_OBJECT_SNAPSHOT_SET, TRUE, 
            &cDeletedSnapshots, &nonDeletedSnapshotId); 
    }
}

bool Confirm(LPCTSTR message)
{
    Console::Write(message); 
    Console::Write(TEXT(" Proceed? [y/N] ")); 
    TCHAR response = Console::ReadChar(); 

    return (response == TEXT('Y') || response == TEXT('y')); 
}

BOOL WINAPI CtrlHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
    case CTRL_C_EVENT:
        InstrumentationHelper::Log(TEXT("Ctrl-C event received. Shutting down."), LOG_LEVEL_WARN); 
        break;
    case CTRL_BREAK_EVENT:
        InstrumentationHelper::Log(TEXT("Ctrl-Break event received. Shutting down."), LOG_LEVEL_WARN); 
        break;
    case CTRL_CLOSE_EVENT:
        InstrumentationHelper::Log(TEXT("Application is being closed. Shutting down."), LOG_LEVEL_WARN); 
        break;
    case CTRL_LOGOFF_EVENT:
        InstrumentationHelper::Log(TEXT("User is logging off. Shutting down."), LOG_LEVEL_WARN); 
        break;
    case CTRL_SHUTDOWN_EVENT:
        InstrumentationHelper::Log(TEXT("System is shutting down. Terminating copy."), LOG_LEVEL_WARN); 
        break;
    }

    s_cancel = true; 

    return TRUE; 

}
void DeleteRecursive(LPCTSTR target)
{
    if (!Utilities::DirectoryExists(target))
    {
        CString message; 
        message.Format(TEXT("Cannot delete directory %s because it does not exist. Proceeding anyway."), 
            target); 
        InstrumentationHelper::Log(message, LOG_LEVEL_WARN); 
        return; 
    }

    CDeleteRecursive deleteAction(target); 
    RecurseDirectory(target, deleteAction, TEXT("")); 
}
void RecurseDirectory(LPCTSTR srcbase, CRecursiveAction& action, LPCTSTR directory)
{
    WIN32_FIND_DATA findData;
    HANDLE hFindHandle;

    CString srcdir;
    Utilities::CombinePath(srcbase, directory, srcdir);

    action.VisitDirectoryInitial(directory); 
    
    CString pattern;
    Utilities::CombinePath(srcdir, TEXT("*"), pattern);

    hFindHandle = ::FindFirstFile(pattern, &findData);
    if (hFindHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                if (Utilities::AreEqual(findData.cFileName, L".") || Utilities::AreEqual(findData.cFileName, L".."))
                {
                    // Do nothing
                }
                else
                {
                    CString subdirectory;
                    Utilities::CombinePath(directory, findData.cFileName, subdirectory);
                    RecurseDirectory(srcbase, action, subdirectory);
                }
            }
            else
            {
                CString file; 
                Utilities::CombinePath(directory, findData.cFileName, file);
                action.VisitFile(file); 
            }
        } while (::FindNextFile(hFindHandle, &findData));

    }
    ::FindClose(hFindHandle);

    // Important to put this after FindClose, since otherwise there's still an 
    // open handle to the directory, and that can interfere with (e.g.) directory
    // deletion
    action.VisitDirectoryFinal(directory); 

}

