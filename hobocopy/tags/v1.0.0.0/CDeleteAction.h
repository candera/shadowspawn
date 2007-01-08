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

#pragma once

#include "Utilities.h"
#include "OutputWriter.h"

class CDeleteAction : public CDirectoryAction
{
private:
    LPCTSTR _target; 

public: 
    CDeleteAction::CDeleteAction(LPCTSTR target)
    {
        _target = target; 
    }

    void VisitDirectoryFinal(LPCTSTR path)
    {
        CString fullPath;
        Utilities::CombinePath(_target, path, fullPath);
        Utilities::FixLongFilenames(fullPath); 

        BOOL bWorked = ::RemoveDirectory(fullPath); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError(); 

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Error %s calling RemoveDirectory on %s"), errorMessage, fullPath); 
            throw new CHoboCopyException(message); 
        }
        else
        {
            CString message; 
            message.AppendFormat(TEXT("Deleted directory %s"), fullPath); 
            OutputWriter::WriteLine(message, VERBOSITY_THRESHOLD_NORMAL); 
        }
    }
  
    void VisitDirectoryInitial(LPCTSTR path)
    {
        // Do nothing
    }

    virtual void VisitFile(LPCTSTR path)
    {
        CString fullPath;
        Utilities::CombinePath(_target, path, fullPath);

        Utilities::FixLongFilenames(fullPath); 

        BOOL bWorked = ::DeleteFile(fullPath); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError(); 

            // Maybe it's read-only
            if (error == 5)
            {
                CString message; 
                message.AppendFormat(TEXT("Permission denied when deleting file %s. Resetting read-only bit and retrying."), 
                    fullPath); 
                OutputWriter::WriteLine(message, VERBOSITY_THRESHOLD_IF_VERBOSE); 

                DWORD attributes = ::GetFileAttributes(fullPath); 

                if (attributes == INVALID_FILE_ATTRIBUTES)
                {
                    CString message;
                    message.AppendFormat(TEXT("Failed to retrieve attributes for file %s."), fullPath); 
                    throw new CHoboCopyException(message); 
                }

                attributes &= ~FILE_ATTRIBUTE_READONLY; 

                bWorked = ::SetFileAttributes(fullPath, attributes); 

                if (!bWorked)
                {
                    CString message;
                    message.AppendFormat(TEXT("Failed to clear read-only bit on %s"), fullPath); 
                    throw new CHoboCopyException(message); 
                }

                bWorked = ::DeleteFile(fullPath); 
                if (!bWorked)
                {
                    error = ::GetLastError(); 
                }
            }

            if (!bWorked)
            {
                CString errorMessage; 
                Utilities::FormatErrorMessage(error, errorMessage); 
                CString message; 
                message.AppendFormat(TEXT("Error %s calling DeleteFile on %s"), errorMessage, path); 
                throw new CHoboCopyException(message); 
            }
        }

        if (bWorked)
        {
            CString message; 
            message.AppendFormat(TEXT("Successfully deleted file %s."), fullPath); 
            OutputWriter::WriteLine(message); 
        }
    }

};