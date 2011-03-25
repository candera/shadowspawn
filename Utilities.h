/* 
Copyright (c) 2011 Wangdera Corporation (hobocopy@wangdera.com)

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

#pragma once

using namespace std; 

#include "CHoboCopyException.h"

class Utilities
{
public: 
    //static LPCTSTR AllocateString(CString& s)
    //{
    //    int length = s.GetLength(); 
    //    LPTSTR sz = new TCHAR[length + 1]; 
    //    s.CopyChars(sz, length, s.GetBuffer(), length); 
    //    sz[length] = TEXT('\0');
    //    return sz; 
    //}
    static bool AreEqual(LPCTSTR wsz1, LPCTSTR wsz2)
    {
        CString s1(wsz1); 
        CString s2(wsz2); 
        return (s1.Compare(wsz2) == 0); 
    }
    static void CombinePath(LPCTSTR wszPath1, LPCTSTR wszPath2, CString& output)
    {
        output.Empty(); 
        output.Append(wszPath1); 

        if (output.GetLength() > 0)
        {
            if (!EndsWith(wszPath1, MAX_PATH, TEXT('\\')))
            {
                output.Append(TEXT("\\")); 
            }
        }

        output.Append(wszPath2); 
    }

    static LPCSTR ConvertToMultibyteString(LPCTSTR s)
    {
        CString s2(s); 
        LPSTR mbBuffer = new CHAR[s2.GetLength() + 1];
#ifdef _UNICODE
        int result = ::WideCharToMultiByte(CP_OEMCP, 0, s2, s2.GetLength(), mbBuffer, s2.GetLength(), NULL, NULL); 

        if (result == 0)
        {
            return NULL; 
        }

        return mbBuffer; 
#else
        return NULL; 
#endif
    }

	/* Test cases: 
		\\server\share\path\to\dir
		C:\path\to\dir
		C:\ - does nothing
		\\server\share - does nothing
		\path\to\dir - use current drive
	*/
    static void CreateDirectory(LPCTSTR directory)
    {
        vector<CString> pathComponents; 
        CString path(directory); 
        GetPathComponents(path, pathComponents); 

        CString pathToCreate; 
		unsigned int initialIndex = 0; 
		bool isUNC = path.Left(2) == _T("\\\\");

		if (isUNC)
		{
			initialIndex = 2;
			pathToCreate.Append(__T("\\\\"));
			pathToCreate.Append(pathComponents[0]);
			pathToCreate.AppendChar(__T('\\'));
			pathToCreate.Append(pathComponents[1]);
			pathToCreate.AppendChar(__T('\\'));
		}

        for (unsigned int iComponent = initialIndex; iComponent < pathComponents.size(); ++iComponent)
        {
            pathToCreate.Append(pathComponents[iComponent]); 
            pathToCreate.AppendChar(TEXT('\\')); 

            if (!DirectoryExists(pathToCreate))
            {
				CString fixedPath(pathToCreate);
				FixLongFilenames(fixedPath);
                BOOL bWorked = ::CreateDirectory(fixedPath, NULL);

                if (!bWorked)
				{
                    DWORD error = ::GetLastError(); 
                    CString errorMessage; 
                    FormatErrorMessage(error, errorMessage); 
                    CString message;
                    message.AppendFormat(TEXT("Failure creating directory %s (as %s) - %s"), pathToCreate, fixedPath, errorMessage); 
                    throw new CHoboCopyException(message); 
                }
            }
        }
    }
    static bool DirectoryExists(LPCTSTR directory)
    {
		CString fixedPath(directory);
		FixLongFilenames(fixedPath);

		WIN32_FILE_ATTRIBUTE_DATA attributes; 
        BOOL bWorked = ::GetFileAttributesEx(fixedPath, GetFileExInfoStandard, &attributes); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError();

            if (error == 2 || error == 3)
            {
                return false; 
            }

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to determine if directory %s (as %s) exists. Error was %s."), 
                directory, fixedPath, errorMessage);
            throw new CHoboCopyException(message); 
        }

        if ((attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
        {
            return attributes.dwFileAttributes != -1;
        }

        return false; 
    }

    static bool EndsWith(LPCTSTR wsz, size_t maxLength, TCHAR wchar)
    {
        CString s(wsz); 

        int length = s.GetLength(); 

        if (length == 0)
        {
            return FALSE; 
        }

        if (s.GetAt(length - 1) == wchar)
        {
            return TRUE; 
        }

        return FALSE; 
    }
    static void FixLongFilenames(CString& path)
    {
        // If it's a UNC path (i.e. starts with \\server), change it to 
        // \\?\UNC\server\path\to\file
        if (Utilities::StartsWith(path, TEXT("\\\\")))
        {
            if (!Utilities::StartsWith(path, TEXT("\\\\?\\")))
            {
                path.Delete(0, 2); 
                path.Insert(0, TEXT("\\\\?\\UNC\\"));
            }
        }
        // Otherwise, change it to \\?\X:\path\to\file
        else 
        {
            if (!Utilities::StartsWith(path, TEXT("\\\\?\\")))
            {
                path.Insert(0, TEXT("\\\\?\\")); 
            }
        }
    }
    static void FormatDateTime(LPSYSTEMTIME utcTime, LPCTSTR separator, bool formatAsLocal, CString& output)
    {
        SYSTEMTIME displayTime; 
        if (formatAsLocal)
        {
            if (::SystemTimeToTzSpecificLocalTime(NULL, utcTime, &displayTime) == 0)
            {
                DWORD error = ::GetLastError(); 
                CString errorMessage; 
                Utilities::FormatErrorMessage(error, errorMessage); 
                CString message; 
                message.AppendFormat(TEXT("Unable to convert UTC time to local time. Error was %s."), 
                    errorMessage); 
                throw new CHoboCopyException(message); 
            }
        }
        else
        {
            displayTime = *utcTime; 
        }

        TCHAR datepart[11]; 
        int result = ::GetDateFormat(LOCALE_USER_DEFAULT, 0, &displayTime, 
            TEXT("yyyy'-'MM'-'dd"), datepart, 11); 

        if (result != 0)
        {
            output.Append(datepart);
            output.Append(separator); 

            TCHAR timepart[9]; 
            result = ::GetTimeFormat(LOCALE_USER_DEFAULT, 0, &displayTime, 
                TEXT("HH':'mm':'ss"), timepart, 9); 

            if (result != 0)
            {
                output.Append(timepart); 
            }
            else
            {
                DWORD error = ::GetLastError(); 
                CString errorMessage; 
                Utilities::FormatErrorMessage(error, errorMessage); 
                output.Empty();
                output.AppendFormat(TEXT("Unable to retrieve time. Error was %s."), errorMessage); 
            }
        }
        else
        {
            DWORD error = ::GetLastError(); 
            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            output.Empty(); 
            output.AppendFormat(TEXT("Unable to retrieve date. Error was %s."), errorMessage); 
        }


    }


    static void FormatErrorMessage(DWORD error, CString& output)
    {
        output.Empty(); 

        LPVOID lpMsgBuf;

        DWORD worked = ::FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            error,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR) &lpMsgBuf,
            0, NULL );

        if (worked > 0)
        {
            CString message; 
            message.Append((LPTSTR) lpMsgBuf); 

            ::LocalFree(lpMsgBuf);

            output.Format(TEXT("%s (Error number %d)"), message, error); 
        }
        else
        {
            output.Format(TEXT("Error %d"), error); 
        }

    }
    static void Free(LPCSTR x)
    {
        delete x; 
    }

    static void GetFileName(CString& path, CString& filename)
    {
        filename.Empty(); 

        vector<CString> pathComponents; 
        GetPathComponents(path, pathComponents); 

        if (pathComponents.size() > 0)
        {
            filename = pathComponents[pathComponents.size() - 1]; 
        }  
    }

    static LONGLONG GetFileSize(LPCTSTR path)
    {
        HANDLE hFile = ::CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL); 

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD error = ::GetLastError(); 

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to open file %s to retrieve file size. Error was %s."), 
                path, errorMessage);
            throw new CHoboCopyException(message); 
        }

        LARGE_INTEGER size; 
        BOOL bWorked = ::GetFileSizeEx(hFile, &size); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError(); 

            ::CloseHandle(hFile);

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to retrieve file size for file %s. Error was %s."), 
                path, errorMessage);
            throw new CHoboCopyException(message); 
        }

        ::CloseHandle(hFile); 

        return size.QuadPart; 
    }

    static void GetPathComponents(CString& path, vector<CString>& pathComponents)
    {
        pathComponents.clear(); 

        bool done = false; 
        int start = 0;
        while (!done)
        {
            CString component = path.Tokenize(TEXT("\\"), start); 

            if (start == -1)
            {
                done = true; 
            }
            else
            {
                pathComponents.push_back(component); 
            }
        }

    }

    static bool IsMatch(CString& input, CString& pattern)
    {
        // Short-circuit common cases
        if (pattern.Compare(TEXT("*")) == 0)
        {
            return true; 
        }

        if (pattern.Compare(TEXT("*.*")) == 0)
        {
            return true; 

        }

        int nStars = 0; 
        for (int iChar = 0; iChar < pattern.GetLength(); ++iChar)
        {
            if (pattern[iChar] == TEXT('*'))
            {
                ++nStars;
            }
        }

        if (nStars > 1)
        {
            CString message; 
            message.AppendFormat(TEXT("The pattern %s is illegal: only a single wildcard is supported."), pattern); 
            throw new CHoboCopyException(message); 
        }

        // No wildcard is present
        if (nStars == 0)
        {
            return input.CompareNoCase(pattern) == 0; 
        }

        // A wildcard is present
        int index = pattern.Find(TEXT("*")); 

        if (index > 0)
        {
            if (index > input.GetLength())
            {
                return false; 
            }

            CString prefix = pattern.Left(index); 
            CString inputStart = input.Left(index); 

            if (prefix.CompareNoCase(inputStart) != 0)
            {
                return false; 
            }
        }

        if (index < input.GetLength() - 1)
        {
            CString suffix = pattern.Mid(index + 1); 
            CString inputEnd = input.Right(suffix.GetLength()); 

            if (suffix.CompareNoCase(inputEnd) != 0)
            {
                return false; 
            }
        }

        return true; 

    }
    //static void FreeString(LPCTSTR s)
    //{
    //    delete s; 
    //}
    static void ParseDateTime(LPCTSTR szDateTime, LPCTSTR separator, LPSYSTEMTIME pTime)
    {
        int separatorLength = CString(separator).GetLength(); 

        CString dateTimeString(szDateTime); 
        pTime->wYear = _ttoi(dateTimeString.Mid(0, 4)); 
        pTime->wMonth = _ttoi(dateTimeString.Mid(5, 2)); 
        pTime->wDay = _ttoi(dateTimeString.Mid(8, 2)); 
        pTime->wHour = _ttoi(dateTimeString.Mid(10 + separatorLength, 2)); 
        pTime->wMinute = _ttoi(dateTimeString.Mid(13 + separatorLength, 2)); 
        pTime->wSecond = _ttoi(dateTimeString.Mid(16 + separatorLength, 2)); 
        pTime->wMilliseconds = 0;
    }
    static bool StartsWith(CString& s1, LPCTSTR s2)
    {
        CString s2a(s2); 
        if (s1.Left(s2a.GetLength()).Compare(s2a) == 0)
        {
            return true; 
        }

        return false; 
    }

};