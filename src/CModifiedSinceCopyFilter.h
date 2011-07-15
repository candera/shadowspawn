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

#include "Utilities.h"
#include "CCopyFilter.h"

class CModifiedSinceCopyFilter : public CCopyFilter
{
private: 
    FILETIME _since; 
	bool _skipDenied; 

public: 
    CModifiedSinceCopyFilter(LPSYSTEMTIME since, bool skipDenied)
    {
		_skipDenied = skipDenied; 
        BOOL worked = ::SystemTimeToFileTime(since, &_since);  

        if (!worked)
        {
            DWORD error = ::GetLastError(); 
            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("SystemTimeToFileTime failed with error %s"), errorMessage); 
            throw new CHoboCopyException(message); 
        }
    }

    bool IsDirectoryMatch(LPCTSTR path)
    {
        return true; 
    }
    
    bool IsFileMatch(LPCTSTR path)
    {
        HANDLE hFile = ::CreateFile(
            path, 
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE, 
            NULL, 
            OPEN_EXISTING, 
            FILE_ATTRIBUTE_NORMAL, 
            NULL); 

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD error = ::GetLastError();

			if (error == 5 && _skipDenied)
			{
				return false; 
			}

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to open file %s exists. Error %s."), path, errorMessage);
            throw new CHoboCopyException(message); 
        }

        FILETIME modified; 
        BOOL worked = ::GetFileTime(hFile, NULL, NULL, &modified); 

        if (!worked)
        {
            DWORD error = ::GetLastError(); 

			if (error == 5 && _skipDenied)
			{
				::CloseHandle(hFile); 
				return false; 
			}

            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to retrieve file time from file %s. Error %s."), path, errorMessage); 
            ::CloseHandle(hFile); 
            throw new CHoboCopyException(message); 
        }

        ::CloseHandle(hFile); 

        int comparison = ::CompareFileTime(&_since, &modified); 

        if (comparison == -1)
        {
            return true;
        }
        else
        {
            return false; 
        }
    }
};