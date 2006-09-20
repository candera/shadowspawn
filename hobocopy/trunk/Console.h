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

class Console
{
private:
    static HANDLE s_hStdOut; 
public: 
    static HANDLE get_StandardOutput(void)
    {
        if (s_hStdOut == INVALID_HANDLE_VALUE)
        {
            HANDLE hOut = ::GetStdHandle(STD_OUTPUT_HANDLE); 

            if (hOut != INVALID_HANDLE_VALUE)
            {
                ::DuplicateHandle(::GetCurrentProcess(), 
                    hOut, 
                    ::GetCurrentProcess(), 
                    &s_hStdOut, 
                    0, 
                    FALSE, 
                    DUPLICATE_SAME_ACCESS); 
            }
        }
        return s_hStdOut;
    }

    static TCHAR ReadChar()
    {
        HANDLE hStdIn = ::GetStdHandle(STD_INPUT_HANDLE); 

        TCHAR buffer[1]; 
        DWORD charsRead; 
        BOOL bWorked = ::ReadConsole(hStdIn, 
            buffer, 
            1, 
            &charsRead, 
            NULL); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError();
            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("There was an error calling ReadConsole. Error %s"), errorMessage); 
            throw new CHoboCopyException(message); 
        }

        if (charsRead != 1)
        {
            throw new CHoboCopyException(TEXT("ReadConsole was unable to read a character.")); 
        }

        return buffer[0]; 
    }

    static void Write(LPCTSTR message)
    {
        CString messageString(message); 

        LPCSTR narrowMessage = Utilities::ConvertToMultibyteString(message); 

        DWORD charsWritten; 
        BOOL bWorked = ::WriteFile(get_StandardOutput(), narrowMessage, messageString.GetLength(), &charsWritten, NULL); 

        Utilities::Free(narrowMessage); 

        if (!bWorked)
        {
            DWORD error = ::GetLastError();
            CString errorMessage; 
            Utilities::FormatErrorMessage(error, errorMessage); 
            CString message; 
            message.AppendFormat(TEXT("Unable to write to the console. %s."), errorMessage); 
            throw new CHoboCopyException(message);             
        }
    }

    static void WriteLine(LPCTSTR message)
    {
        Write(message); 
        Write(TEXT("\r\n")); 
    }
};