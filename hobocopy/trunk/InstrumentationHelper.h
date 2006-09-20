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

using namespace std; 

typedef enum LOG_LEVEL
{
    LOG_LEVEL_FATAL = 0, 
    LOG_LEVEL_ERROR = 1, 
    LOG_LEVEL_WARN = 2, 
    LOG_LEVEL_INFO = 3, 
    LOG_LEVEL_DEBUG = 4
} LOG_LEVEL; 

class InstrumentationHelper
{
private: 
    static LOG_LEVEL s_logLevel; 

    static LPCTSTR LogLevelToString(LOG_LEVEL level)
    {
        switch (level)
        {
        case LOG_LEVEL_FATAL:
            return TEXT("FATAL");
        case LOG_LEVEL_ERROR: 
            return TEXT("ERROR"); 
        case LOG_LEVEL_WARN: 
            return TEXT("WARN "); 
        case LOG_LEVEL_INFO:
            return TEXT("INFO "); 
        case LOG_LEVEL_DEBUG: 
            return TEXT("DEBUG"); 
        default:
            return TEXT("UNKWN"); 
        }
    }
public: 
    static void Log(LPCTSTR message)
    {
        Log(message, LOG_LEVEL_DEBUG); 
    }
    static void Log(LPCTSTR message, LOG_LEVEL level)
    {
        if (InstrumentationHelper::s_logLevel >= level)
        {
            Console::WriteLine(message); 
        }
    }

    static void SetLogLevel(LOG_LEVEL logLevel)
    {
        s_logLevel = logLevel; 
    }
};