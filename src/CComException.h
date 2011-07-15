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

#include "stdafx.h"
#include <string>
#include <iostream>

using namespace std;

class CComException
{

private: 
    HRESULT _hresult; 
    const char* _file; 
    int _line; 

public: 
    CComException::CComException(HRESULT hresult, const char* file, int line)
    {
        _hresult = hresult; 
        _file = file; 
        _line = line; 
    }

    HRESULT get_Hresult(void)
    {
        return _hresult; 
    }

    void get_File(CString& file)
    {
        // Hack: this part is not TCHAR-aware, but this is the only place
        size_t length = strlen(_file); 
        WCHAR* buffer = new WCHAR[length + 1]; 
        ::MultiByteToWideChar(CP_ACP, 0, _file, (int) length, buffer, (int) length); 
        buffer[length] = L'\0';
        file.Empty(); 
        file.Append(buffer); 
        delete buffer; 
    }

    int get_Line(void)
    {
        return _line; 
    }

};