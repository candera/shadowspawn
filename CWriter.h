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

#include "CWriterComponent.h"

using namespace std;

class CWriter
{
private:
    vector<CWriterComponent> _components;
    vector<CWriterComponent> _componentTree; 
    GUID _instanceId;
    CString _name; 
    GUID _writerId; 
    
public:
    vector<CWriterComponent>& get_Components(void)
    {
        return _components; 
    }

    GUID get_InstanceId(void)
    {
        return _instanceId; 
    }

    void set_InstanceId(GUID& value)
    {
        _instanceId = value;
    }

    CString& get_Name(void)
    {
        return _name; 
    }

    void set_Name(CString name)
    {
        _name = name; 
    }

    GUID get_WriterId(void)
    {
        return _writerId;
    }

    void set_WriterId(GUID& value)
    {
        _writerId = value; 
    }

    void ComputeComponentTree(void)
    {
        for (unsigned int iComponent = 0; iComponent < _components.size(); ++iComponent)
        {
            CWriterComponent& current = _components[iComponent];

            for (unsigned int iComponentParent = 0; iComponentParent < _components.size(); ++iComponentParent)
            {
                if (iComponentParent == iComponent)
                {
                    continue; 
                }

                CWriterComponent& potentialParent = _components[iComponentParent]; 
                if (potentialParent.IsParentOf(current))
                {
                    current.set_Parent(&potentialParent); 
                    break; 
                }
            }
        }

    }
};