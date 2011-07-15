/* 
Copyright (c) 2011 Craig Andera (shadowspawn@wangdera.com)

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

#include <assert.h>
#include "CShadowSpawnException.h"
#include "CParseOptionsException.h"
#include "Utilities.h"
#include "OutputWriter.h"

using namespace std; 

class COptions
{
private: 
    wstring _command;
    bool _debug; 
    CString _destination; 
    bool _simulate; 
    CString _source; 
    int _verbosityLevel;

public: 
    VSS_BACKUP_TYPE get_BackupType()
    {
        return VSS_BACKUP_TYPE::VSS_BT_FULL; 
    }
    const wstring& get_Command(void)
    {
        return _command; 
    }    
    bool get_Debug(void)
    {
        return _debug; 
    }
    LPCTSTR get_Destination(void)
    {
        return _destination.GetString(); 
    }
    bool get_Simulate(void)
    {
        return _simulate; 
    }
    LPCTSTR get_Source(void)
    {
        return _source.GetString(); 
    }
    static LPCTSTR get_Usage(void)
    {
        return TEXT("Usage:\n\n")
            TEXT("shadowspawn [ /verbosity=LEVEL ] <src> <drive:> <command> [ <arg> ... ]\n")
            TEXT("\n")
            TEXT("Creates a shadow copy of <src>, mounts it at <drive:> and runs <command>.\n")
            TEXT("\n")
            TEXT("/verbosity   - Specifies how much information ShadowSpawn will emit\n")
            TEXT("               during execution. Legal values are: 0 - almost no\n")
            TEXT("               information will be emitted. 1 - Only error information\n")
            TEXT("               will be emitted. 2 - Errors and warnings will be\n")
            TEXT("               emitted. 3 - Errors, warnings, and some status\n")
            TEXT("               information will be emitted. 4 - Lots of diagnostic\n")
            TEXT("               information will be emitted. The default level is 2.\n")
            TEXT("\n")
            TEXT("<src>        - The directory to shadow copy (the source directory).\n")
            TEXT("<drive:>     - Where to mount the shadow copy. Must be a single letter\n")
            TEXT("               followed by a colon. E.g. 'X:'. The drive letter must be\n")
            TEXT("               available (i.e. nothing else mounted there).\n")
            TEXT("<command>    - A command to run. ShadowSpawn will ensure that <src> is\n")
            TEXT("               mounted at <drive:> before starting <command>, and will\n")
            TEXT("               wait for <command> to finish before unmounting <drive:>\n")
            ; 
    }
    int get_VerbosityLevel(void)
    {
        return _verbosityLevel; 
    }

    static COptions Parse(int argc, _TCHAR* argv[])
    {
        COptions options;
        
        options._verbosityLevel = VERBOSITY_LEVEL_NORMAL; 
        options._debug = false; 
        options._simulate = false; 

        vector<wstring> tokens;
        COptions::RawParse(tokens);
        
        for (int i = 1; i < tokens.size(); ++i)
        {
            CString arg(tokens[i].c_str()); 
            arg.MakeLower();

            if (options._command.empty() && Utilities::StartsWith(arg, TEXT("/")) || Utilities::StartsWith(arg, TEXT("-")))
            {
                arg = arg.Mid(1);

                if (Utilities::StartsWith(arg, TEXT("verbosity=")))
                {
                    options._verbosityLevel = _ttoi(GetArgValue(arg)); 
                }
                else if (arg.Compare(TEXT("debug")) == 0)
                {
                    options._debug = true; 
                }
                else if (arg.Compare(TEXT("simulate")) == 0)
                {
                    options._simulate = true; 
                }
                else
                {
                    assert(false);
                    CString message("Unrecognized switch: "); 
                    message.Append(arg); 
                    throw new CParseOptionsException(message); 
                }
            }
            else
            {
                if (options._source.IsEmpty())
                {
                    options._source = tokens[i].c_str(); 
                }
                else if (options._destination.IsEmpty())
                {
                    options._destination = tokens[i].c_str(); 
                }
                else
                {
                    options._command.append(tokens[i].c_str());
                    options._command.append(TEXT(" "));
                }
            }
        }

        // Normalize paths to full paths
        options._source = NormalizePath(options._source); 
        options._destination = NormalizePath(options._destination); 

        if (options._source.IsEmpty() || options._destination.IsEmpty() || options._command.empty())
        {
            throw new CParseOptionsException(TEXT("Missing required arguments.")); 
        }

        return options; 
    }

private:
    static CString GetArgValue(const CString& arg)
    {
        int index = arg.Find(L'='); 

        if (index == -1)
        {
            CString message; 
            message.Format(TEXT("Couldn't parse the option value from %s"), 
                arg.GetString()); 
            throw new CParseOptionsException(message.GetString()); 
        }

        return arg.Mid(index + 1); 
    }
    static CString NormalizePath(LPCTSTR path)
    {
        int length = MAX_PATH; 
        while (true)
        {
            _TCHAR* normalizedPath = new _TCHAR[length]; 
            LPTSTR filePart; 
            DWORD result = ::GetFullPathName(path, MAX_PATH, normalizedPath, &filePart);

            if (result == 0)
            {
                DWORD error = ::GetLastError(); 
                CString errorMessage; 
                Utilities::FormatErrorMessage(error, errorMessage); 
                CString message; 
                message.AppendFormat(TEXT("Error calling GetFullPathName: %s"), errorMessage); 
                throw new CShadowSpawnException(message.GetString()); 
            }
            else if (result > MAX_PATH)
            {
                delete normalizedPath; 
                length = result; 
            }
            else
            {
                return normalizedPath; 
            }
        }
    }
    static LPCTSTR NullIfEmpty(CString& string)
    {
        if (string.GetLength() == 0)
        {
            return NULL; 
        }
        else
        {
            return string.GetString(); 
        }

    }
    static wregex* ParseRegex(const CString& userInput) 
    {
        try
        {
            return new wregex(userInput.GetString());
        }
        catch (regex_error err)
        {
            switch (err.code()) 
            {
            case regex_constants::error_badbrace:
                ThrowRegexParseException(TEXT("The expression contained an invlid count in a { } expression"));			
                break;

            case regex_constants::error_badrepeat:
                ThrowRegexParseException(TEXT("A repeat expression (one of '*', '?', '+', '{' in most contexts) was not preceded by an expression"));
                break;

            case regex_constants::error_brace:
                ThrowRegexParseException(TEXT("The expression contained an unmatched '{' or '}'"));
                break;

            case regex_constants::error_brack:
                ThrowRegexParseException(TEXT("The expression contained an unmatched '[' or ']'"));
                break;

            case regex_constants::error_collate:
                ThrowRegexParseException(TEXT("The expression contained an invalid collating element name"));
                break;

            case regex_constants::error_complexity:
                ThrowRegexParseException(TEXT("An attempted match failed because it was too complex"));
                break;

            case regex_constants::error_ctype:
                ThrowRegexParseException(TEXT("The expression contained an invalid character class name"));
                break;

            case regex_constants::error_escape:
                ThrowRegexParseException(TEXT("The expression contained an invalid escape sequence"));
                break;

            case regex_constants::error_paren:
                ThrowRegexParseException(TEXT("The expression contained an unmatched '(' or ')'"));
                break;

            case regex_constants::error_range:
                ThrowRegexParseException(TEXT("The expression contained an invalid character range specifier"));
                break;

            case regex_constants::error_space:
                ThrowRegexParseException(TEXT("Parsing a regular expression failed because there were not enough resources available"));
                break;

            case regex_constants::error_stack:
                ThrowRegexParseException(TEXT("An attempted match failed because there was not enough memory available"));
                break;

            case regex_constants::error_backref:
                ThrowRegexParseException(TEXT("The expression contained an invalid back reference"));
                break;

            default:                                
                ThrowRegexParseException(TEXT("Parse failed"));							
                break;
            }
        }

        ThrowRegexParseException(TEXT("Parse failed"));							
        return NULL; 

    }
    static void RawParse(vector<wstring>& tokens)
    {
        wchar_t* lp = GetCommandLineW();
        while (*lp != L'\0')
        {
            if (*lp == L'\"')
            {
                wstring token;
                token.assign(1, L'\"');
                lp++;
                while (*lp != L'\"' && *lp != L'\0')
                {
                    token.append(1, *lp);
                    lp++;
                }
                lp++;
                token.append(1, L'\"');
                tokens.push_back(token);
            }
            else if (*lp == L' ')
            {
                wstring token;
                while (*lp == L' ') lp++;
                while (*lp != L' ' && *lp != L'\0')
                {
                    token.append(1, *lp);
                    lp++;
                }
                if (!token.empty()) tokens.push_back(token);
            }
            else  // start of command line when outside of debugger
            {
                wstring token;
                assert(tokens.empty());
                while (*lp != L' ' && *lp != L'\0')
                {
                    token.append(1, *lp);
                    lp++;
                }
                if (!token.empty()) tokens.push_back(token);
            }
        }
    }
    static void ThrowRegexParseException(const TCHAR* pszDetails) 
    {
        CString message;
        message.Format(TEXT("Couldn't parse regular expression supplied to /ignorepattern option (%s)"), pszDetails);
        throw new CParseOptionsException(message); 
    }
};
