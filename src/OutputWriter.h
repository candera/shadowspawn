/* 
Copyright (c) 2011 Wangdera Corporation (shadowspawn@wangdera.com)

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

#include "Console.h"

using namespace std; 

typedef enum VERBOSITY_LEVEL
{
    VERBOSITY_LEVEL_SILENT = 1, 
    VERBOSITY_LEVEL_TERSE = 2, 
    VERBOSITY_LEVEL_NORMAL = 3, 
    VERBOSITY_LEVEL_VERBOSE = 4, 
} VERBOSITY_LEVEL; 

typedef enum VERBOSITY_THRESHOLD
{
    VERBOSITY_THRESHOLD_ALWAYS = 1,
    VERBOSITY_THRESHOLD_UNLESS_SILENT = 2,
    VERBOSITY_THRESHOLD_NORMAL = 3,
    VERBOSITY_THRESHOLD_IF_VERBOSE = 4,
} VERBOSITY_THRESHOLD;

class OutputWriter
{
private: 
    static VERBOSITY_LEVEL s_verbosityLevel; 

/*
    static LPCTSTR LogLevelToString(VERBOSITY_LEVEL level)
    {
        switch (level)
        {
        case LOG_LEVEL_FATAL:
            return TEXT("FATAL");
        case VERBOSITY_THRESHOLD_UNLESS_SILENT: 
            return TEXT("ERROR"); 
        case VERBOSITY_THRESHOLD_NORMAL: 
            return TEXT("WARN "); 
        case VERBOSITY_THRESHOLD_NORMAL:
            return TEXT("INFO "); 
        case VERBOSITY_THRESHOLD_IF_VERBOSE: 
            return TEXT("DEBUG"); 
        default:
            return TEXT("UNKWN"); 
        }
    }
*/
public: 
    static void WriteLine(LPCTSTR message)
    {
        WriteLine(message, VERBOSITY_THRESHOLD_IF_VERBOSE); 
    }
    static void WriteLine(LPCTSTR message, VERBOSITY_THRESHOLD threshold)
    {
        if (OutputWriter::s_verbosityLevel >= threshold)
        {
            Console::WriteLine(message); 
        }
    }

    static void SetVerbosityLevel(VERBOSITY_LEVEL verbosityLevel)
    {
        s_verbosityLevel = verbosityLevel; 
    }
};