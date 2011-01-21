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

#include "CHoboCopyException.h"
#include "CParseOptionsException.h"
#include "Utilities.h"
#include "OutputWriter.h"

using namespace std; 

class COptions
{
private: 
    bool _acceptAll; 
    VSS_BACKUP_TYPE _backupType; 
    bool _clearDestination;
    bool _debug; 
    CString _destination; 
    vector<CString> _filespecs; 
    Regex* _ignorePattern;
    bool _recursive; 
    bool _simulate; 
    bool _skipDenied; 
    CString _source; 
    CString _stateFile; 
    int _verbosityLevel;

public: 
    bool get_AcceptAll()
    {
        return _acceptAll; 
    }
    VSS_BACKUP_TYPE get_BackupType()
    {
        return _backupType; 
    }
    bool get_ClearDestination()
    {
        return _clearDestination; 
    }
    bool get_Debug(void)
    {
        return _debug; 
    }
    LPCTSTR get_Destination(void)
    {
        return _destination.GetString(); 
    }
    vector<CString>& get_Filespecs(void)
    {
        return _filespecs; 
    }
    Regex* get_IgnorePattern(void)
    {
        return _ignorePattern;
    }
    bool get_Recursive(void)
    {
        return _recursive; 
    }
    bool get_Simulate(void)
    {
        return _simulate; 
    }
    bool get_SkipDenied(void)
    {
        return _skipDenied; 
    }
    LPCTSTR get_StateFile()
    {
        return NullIfEmpty(_stateFile); 
    }
    LPCTSTR get_Source(void)
    {
        return _source.GetString(); 
    }
    static LPCTSTR get_Usage(void)
    {
        return TEXT("Usage:\n\n")
            TEXT("hobocopy [/statefile=FILE] [/verbosity=LEVEL] [/ignorepattern=REGEX]\n")
            TEXT("         [ /full | /incremental ] [ /clear ] [ /skipdenied ] [ /y ]\n")
            TEXT("         [ /simulate ] [/recursive]\n")
            TEXT("         <src> <dest> [<file> [<file> [ ... ] ]\n")
            TEXT("\n")
            TEXT("Recursively copies a directory tree from <src> to <dest>.\n")
            TEXT("\n")
            TEXT("/statefile   - Specifies a file where information about the copy will\n")
            TEXT("               be written. This argument is required when /incremental\n")
            TEXT("               is specified, as the date and time of the last copy is\n")
            TEXT("               read from this file to determine which files should be\n")
            TEXT("               copied.\n")
            TEXT("\n")
            TEXT("/verbosity   - Specifies how much information HoboCopy will emit\n")
            TEXT("               during copy. Legal values are: 0 - almost no\n")
            TEXT("               information will be emitted. 1 - Only error information\n")
            TEXT("               will be emitted. 2 - Errors and warnings will be\n")
            TEXT("               emitted. 3 - Errors, warnings, and some status\n")
            TEXT("               information will be emitted. 4 - Lots of diagnostic\n")
            TEXT("               information will be emitted. The default level is 2.\n")
            TEXT("\n")
            TEXT("/full        - Perform a full copy. All files will be copied\n")
            TEXT("               regardless of modification date.\n")
            TEXT("\n")
            TEXT("/incremental - Perform an incremental copy. Only files that have\n")
            TEXT("               changed since the last full copy will be copied.\n")
            TEXT("               Specifying this switch requires the /statefile switch\n")
            TEXT("               to be specified, as that's where the date of the last\n")
            TEXT("               full copy is read from.\n")
            TEXT("\n")
            TEXT("/clear       - Recursively delete the destination directory before\n")
            TEXT("               copying. HoboCopy will ask for confirmation before\n")
            TEXT("               deleting unless the /y switch is also specified.\n")
            TEXT("\n")
            TEXT("/skipdenied  - By default, if HoboCopy does not have sufficient\n")
            TEXT("               privilege to copy a file, the copy will fail with an\n")
            TEXT("               error. When the /skipdenied switch is specified,\n")
            TEXT("               permission errors trying to copy a source file result\n")
            TEXT("               in the file being skipped and the copy continuing.\n")
            TEXT("\n")
            TEXT("/y           - Instructs HoboCopy to proceed as if user answered yes\n")
            TEXT("               to any confirmation prompts. Use with caution - in\n")
            TEXT("               combination with the /clear switch, this switch will\n")
            TEXT("               cause the destination directory to be deleted without\n")
            TEXT("               confirmation.\n")
            TEXT("\n")
            TEXT("/simulate    - Simulates copy only - no snapshot is taken and no copy\n")
            TEXT("               is performed.\n")
            TEXT("\n")
            TEXT("/recursive   - Copies subdirectories (including empty ones). Shortcut: /r\n")
            TEXT("\n")
            TEXT("/ignorepattern - This is a regular expression that allows you to\n")
            TEXT("                 designate files or directories that HoboCopy should\n")
            TEXT("                 not copy nor recurse into.\n")
            TEXT("\n")
            TEXT("<src>        - The directory to copy (the source directory).\n")
            TEXT("<dest>       - The directory to copy to (the destination directory).\n")
            TEXT("<file>       - A file (e.g. foo.txt) or filespec (e.g. *.txt) to copy.\n")
            TEXT("               Defaults to *.*.\n"); 

        //TEXT("\n")
        //TEXT("\n")
        ; 
    }
    int get_VerbosityLevel(void)
    {
        return _verbosityLevel; 
    }

    static COptions Parse(int argc, _TCHAR* argv[])
    {
        COptions options;

        options._backupType = VSS_BT_FULL;
        options._clearDestination = false; 
        options._verbosityLevel = VERBOSITY_LEVEL_NORMAL; 
        options._acceptAll = false; 
        options._skipDenied = false; 
        options._debug = false; 
        options._simulate = false; 
        options._recursive = false; 
        options._ignorePattern = NULL;

        if (argc < 3)
        {
            throw new CParseOptionsException(TEXT("Wrong number of arguments.")); 
        }

        for (int i = 1; i < argc; ++i)
        {
            CString arg(argv[i]); 
            arg.MakeLower();

            if (Utilities::StartsWith(arg, TEXT("/")) || Utilities::StartsWith(arg, TEXT("-")))
            {
                arg = arg.Mid(1);

                if (arg.Compare(TEXT("full")) == 0)
                {
                    options._backupType = VSS_BT_FULL; 
                }
                else if (arg.Compare(TEXT("incremental")) == 0)
                {
                    options._backupType = VSS_BT_INCREMENTAL; 
                }
                else if (arg.Compare(TEXT("clear")) == 0)
                {
                    options._clearDestination = true; 
                }
                else if (Utilities::StartsWith(arg, TEXT("statefile=")))
                {
                    options._stateFile = GetArgValue(arg); 
                }
                else if (Utilities::StartsWith(arg, TEXT("verbosity=")))
                {
                    options._verbosityLevel = _ttoi(GetArgValue(arg)); 
                }
                else if (Utilities::StartsWith(arg, TEXT("ignorepattern=")))
                {
                    options._ignorePattern = ParseRegex(GetArgValue(arg));
                }
                else if (arg.Compare(TEXT("y")) == 0)
                {
                    options._acceptAll = true; 
                }
                else if (arg.Compare(TEXT("skipdenied")) == 0)
                {
                    options._skipDenied = true; 
                }
                else if (arg.Compare(TEXT("debug")) == 0)
                {
                    options._debug = true; 
                }
                else if (arg.Compare(TEXT("simulate")) == 0)
                {
                    options._simulate = true; 
                }
                else if (arg.Compare(TEXT("recursive")) == 0 || arg.Compare(TEXT("r")) == 0)
                {
                    options._recursive = true; 
                }
                else
                {
                    CString message("Unrecognized switch: "); 
                    message.Append(arg); 
                    throw new CParseOptionsException(message); 
                }
            }
            else
            {
                if (options._source.IsEmpty())
                {
                    options._source = argv[i]; 
                }
                else if (options._destination.IsEmpty())
                {
                    options._destination = argv[i]; 
                }
                else
                {
                    options._filespecs.push_back(argv[i]); 
                }
            }
        }

        // Normalize paths to full paths
        options._source = NormalizePath(options._source); 
        options._destination = NormalizePath(options._destination); 

        if (!options._stateFile.IsEmpty())
        {
            options._stateFile = NormalizePath(options._stateFile); 
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
                throw new CHoboCopyException(message.GetString()); 
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
    static Regex* ParseRegex(const CString& userInput) 
    {
        Regex* regex = new Regex();
        switch (regex->Parse(userInput.GetString(), FALSE)) 
        {
            case REPARSE_ERROR_OK:
                return regex;
            case REPARSE_ERROR_OUTOFMEMORY:			
                ThrowRegexParseException(TEXT("Out of memory"));						
                break;
            case REPARSE_ERROR_BRACE_EXPECTED:		
                ThrowRegexParseException(TEXT("A closing brace was expected"));			
                break;
            case REPARSE_ERROR_PAREN_EXPECTED:		
                ThrowRegexParseException(TEXT("A closing parenthesis was expected"));	
                break;
            case REPARSE_ERROR_BRACKET_EXPECTED:	
                ThrowRegexParseException(TEXT("A closing bracket was expected"));		
                break;
            case REPARSE_ERROR_UNEXPECTED:			
                ThrowRegexParseException(TEXT("An unspecified fatal error occurred"));	
                break;
            case REPARSE_ERROR_EMPTY_RANGE:			
                ThrowRegexParseException(TEXT("A range expression was empty"));			
                break;
            case REPARSE_ERROR_INVALID_GROUP:		
                ThrowRegexParseException(TEXT("A backreference was made to a group that did not exist")); 
                break;
            case REPARSE_ERROR_INVALID_RANGE:		
                ThrowRegexParseException(TEXT("An invalid range was specified"));		
                break;
            case REPARSE_ERROR_EMPTY_REPEATOP:		
                ThrowRegexParseException(TEXT("A possibly empty * or + was detected")); 
                break;
            case REPARSE_ERROR_INVALID_INPUT:		
                ThrowRegexParseException(TEXT("The input string was invalid"));			
                break;
            default:                                
                ThrowRegexParseException(TEXT("Parse failed"));							
				break;
        }

		ThrowRegexParseException(TEXT("Parse failed"));							
		return NULL; 

    }
    static void ThrowRegexParseException(const TCHAR* pszDetails) 
    {
        CString message;
        message.Format(TEXT("Couldn't parse regular expression supplied to /ignorepattern option (%s)"), pszDetails);
        throw new CParseOptionsException(message); 
    }
};
