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

#pragma once

using namespace std; 

class CFilespecCopyFilter : public CCopyFilter
{
private:
    vector<CString>& _filespecs; 

public:
    CFilespecCopyFilter::CFilespecCopyFilter(vector<CString>& filespecs) : _filespecs(filespecs)
    {
    }

    bool IsDirectoryMatch(LPCTSTR path)
    {
        return true; 
    }
    bool IsFileMatch(LPCTSTR path)
    {
        // No filespecs means "match everything"
        if (_filespecs.size() == 0)
        {
            return true; 
        }

        CString filename; 
        Utilities::GetFileName(CString(path), filename); 

        for (unsigned int iFilespec = 0; iFilespec < _filespecs.size(); ++iFilespec)
        {
            if (Utilities::IsMatch(filename, _filespecs[iFilespec]))
            {
                return true; 
            }
        }

        return false; 
    }
};
