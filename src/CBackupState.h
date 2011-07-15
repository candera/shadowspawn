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

#include "CComException.h"
#include "OutputWriter.h"

class CBackupState
{
private:
    bool _hasLastFullBackup; 
    bool _hasLastIncrementalBackup; 
    SYSTEMTIME _lastFullBackup; 
    SYSTEMTIME _lastIncrementalBackup; 

    void AppendAttribute(CComPtr<IXMLDOMDocument> document, CComPtr<IXMLDOMElement> parent, 
        LPCTSTR name, LPCTSTR value)
    {
        CString message; 
        message.AppendFormat(TEXT("Creating attribute %s"), name); 
        OutputWriter::WriteLine(message); 
        CComPtr<IXMLDOMAttribute> pAttribute; 
        CHECK_HRESULT(document->createAttribute(CComBSTR(name), &pAttribute)); 

        OutputWriter::WriteLine(TEXT("Setting value of attribute."));
        CHECK_HRESULT(pAttribute->put_text(CComBSTR(value))); 

        OutputWriter::WriteLine(TEXT("Retrieving attributes from parent element.")); 
        CComPtr<IXMLDOMNamedNodeMap> pAttributes;
        CHECK_HRESULT(parent->get_attributes(&pAttributes)); 

        OutputWriter::WriteLine(TEXT("Adding attribute to parent.")); 
        CComPtr<IXMLDOMNode> pThrowawayNode; 
        CHECK_HRESULT(pAttributes->setNamedItem(pAttribute, &pThrowawayNode)); 
    }

    bool SelectDateTimeValue(CComPtr<IXMLDOMDocument> document,
        LPCTSTR xpath, LPSYSTEMTIME pTime)
    {
        CString message(TEXT("Selecting node for xpath ")); 
        message.Append(xpath); 
        OutputWriter::WriteLine(message); 
        CComPtr<IXMLDOMNode> pNode; 
        HRESULT hr = document->selectSingleNode(CComBSTR(xpath), &pNode); 

        if (hr == S_FALSE)
        {
            OutputWriter::WriteLine(TEXT("Unable to find matching node.")); 
            return FALSE; 
        }
        else
        {
            // Cheap way of throwing an exception with the details filled in
            CHECK_HRESULT(hr); 
        }

        OutputWriter::WriteLine(TEXT("Retrieving text value of node")); 
        CComBSTR bstrLastFullBackup; 
        CHECK_HRESULT(pNode->get_text(&bstrLastFullBackup)); 

        CString lastFullBackup(bstrLastFullBackup); 

        Utilities::ParseDateTime(lastFullBackup, TEXT("T"), pTime); 

        CString message2(TEXT("Time value was: ")); 
        CString dateTime; 
        Utilities::FormatDateTime(pTime, TEXT(" "), false, dateTime);
        message2.Append(dateTime); 
        OutputWriter::WriteLine(message2); 

        return true; 
    }

public: 
    CBackupState::CBackupState(void)
    {
        _hasLastFullBackup = false; 
        _hasLastIncrementalBackup = false; 
    }

    LPSYSTEMTIME get_LastFullBackupTime()
    {
        if (_hasLastFullBackup)
        {
            return &_lastFullBackup; 
        }
        else
        {
            return NULL; 
        }
    }

    void set_LastFullBackupTime(LPSYSTEMTIME value)
    {
        _hasLastFullBackup = true; 
        _lastFullBackup = *value; 
    }

    LPSYSTEMTIME get_LastIncrementalBackupTime()
    {
        if (_hasLastIncrementalBackup)
        {
            return &_lastIncrementalBackup; 
        }
        else
        {
            return NULL; 
        }
    }

    void set_LastIncrementalBackupTime(LPSYSTEMTIME value) 
    {
        _hasLastIncrementalBackup = true; 
        _lastIncrementalBackup = *value; 
    }

    void Load(LPCTSTR path)
    {
        _hasLastFullBackup = false; 
        _hasLastIncrementalBackup = false; 

        OutputWriter::WriteLine(TEXT("Creating DOM document object.")); 
        CComPtr<IXMLDOMDocument> stateDocument; 
        CHECK_HRESULT(stateDocument.CoCreateInstance(__uuidof(DOMDocument30))); 

        CString message; 
        message.AppendFormat(TEXT("Loading state file from %s."), path); 
        OutputWriter::WriteLine(message); 

        OutputWriter::WriteLine(TEXT("Turning off validation")); 
        CHECK_HRESULT(stateDocument->put_validateOnParse(VARIANT_FALSE)); 

        VARIANT_BOOL isSuccessful; 
        HRESULT hr = stateDocument->load(CComVariant(path), &isSuccessful); 

        if (FAILED(hr))
        {
            // A cheap way of throwing an exception with the details filled in
            CHECK_HRESULT(hr); 
        }
        else if (hr == S_FALSE || isSuccessful == VARIANT_FALSE)
        {
            CComPtr<IXMLDOMParseError> pError; 
            CHECK_HRESULT(stateDocument->get_parseError(&pError)); 
            CComBSTR bstrReason; 
            CHECK_HRESULT(pError->get_reason(&bstrReason)); 
            CString reason(bstrReason); 
            CString message; 
            message.AppendFormat(TEXT("Failed to load state file from %s. Reason: %s."), path, (LPCTSTR) reason); 
            throw new CShadowSpawnException(message); 
        }

        _hasLastFullBackup = SelectDateTimeValue(stateDocument, TEXT("/shadowSpawnState/@lastFullBackup"), &_lastFullBackup); 

        if (_hasLastFullBackup)
        {
            CString message; 
            CString dateTime; 
            Utilities::FormatDateTime(&_lastFullBackup, TEXT(" "), false, dateTime);
            message.AppendFormat(TEXT("Last full backup time read as %s"), dateTime); 
            OutputWriter::WriteLine(message); 
        }
        else
        {
            OutputWriter::WriteLine(TEXT("Backup file did not have last full backup time recorded.")); 
        }

        _hasLastIncrementalBackup = SelectDateTimeValue(stateDocument, TEXT("/shadowSpawnState/@lastIncrementalBackup"), &_lastIncrementalBackup); 

        if (_hasLastIncrementalBackup)
        {
            CString message; 
            CString dateTime; 
            Utilities::FormatDateTime(&_lastIncrementalBackup, TEXT(" "), false, dateTime);
            message.AppendFormat(TEXT("Last incremental backup time read as %s"), dateTime); 
            OutputWriter::WriteLine(message); 
        }
        else
        {
            OutputWriter::WriteLine(TEXT("Backup file did not have last incremental backup time recorded.")); 
        }
    }

    void Save(LPCTSTR path, BSTR backupDocument)
    {
        if (!_hasLastFullBackup)
        {
            throw new CShadowSpawnException(TEXT("Could not locate last full backup time.")); 
        }

        CString message; 
        message.AppendFormat(TEXT("Saving backup document to state file %s"), path); 
        OutputWriter::WriteLine(message, VERBOSITY_THRESHOLD_NORMAL); 

        OutputWriter::WriteLine(TEXT("Backup document:")); 
        OutputWriter::WriteLine(backupDocument); 

        OutputWriter::WriteLine(TEXT("Creating DOM document object.")); 
        CComPtr<IXMLDOMDocument> backupDocumentDom; 
        //IXMLDOMDocument* backupDocumentDom; 
        //CHECK_HRESULT(::CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC, IID_IXMLDOMDocument, (LPVOID*) &backupDocumentDom) 
        CHECK_HRESULT(backupDocumentDom.CoCreateInstance(__uuidof(DOMDocument30))); 

        OutputWriter::WriteLine(TEXT("Turning off validation")); 
        CHECK_HRESULT(backupDocumentDom->put_validateOnParse(VARIANT_FALSE)); 

        OutputWriter::WriteLine(TEXT("Loading backup document into DOM.")); 
        VARIANT_BOOL worked; 
        HRESULT hr = backupDocumentDom->loadXML(backupDocument, &worked);

        if (FAILED(hr))
        {
            CString message; 
            message.Format(TEXT("loadXML failed with HRESULT 0x%x"), hr); 
            throw new CShadowSpawnException(message); 
        }
        else if (hr == S_FALSE)
        {
            CComPtr<IXMLDOMParseError> parseError; 
            OutputWriter::WriteLine(TEXT("Retrieving parse error")); 
            CHECK_HRESULT(backupDocumentDom->get_parseError(&parseError)); 
            CComBSTR bstrReason; 
            OutputWriter::WriteLine(TEXT("Retrieving reason")); 
            CHECK_HRESULT(parseError->get_reason(&bstrReason));
            CString message; 
            message.Format(TEXT("loadXML failed to parse: %s"), bstrReason); 
            throw new CShadowSpawnException(message); 
        }

        if (worked == VARIANT_FALSE)
        {
            throw new CShadowSpawnException(TEXT("IXMLDOMDocument::loadXML() failed")); 
        }

        OutputWriter::WriteLine(TEXT("Creating state element.")); 
        CComPtr<IXMLDOMElement> pStateElement; 
        CHECK_HRESULT(backupDocumentDom->createElement(CComBSTR(TEXT("shadowSpawnState")), 
            &pStateElement)); 

        if (_hasLastFullBackup)
        {
            OutputWriter::WriteLine(TEXT("Creating lastFullBackup attribute")); 
            CString dateTime; 
            Utilities::FormatDateTime(&_lastFullBackup, TEXT("T"), false, dateTime);
            AppendAttribute(backupDocumentDom, pStateElement, TEXT("lastFullBackup"), dateTime); 
        }
        else
        {
            OutputWriter::WriteLine(TEXT("No last full backup time was present.")); 
        }

        if (_hasLastIncrementalBackup)
        {
            OutputWriter::WriteLine(TEXT("Creating lastIncrementalBackup attribute")); 
            CString dateTime; 
            Utilities::FormatDateTime(&_lastIncrementalBackup, TEXT("T"), false, dateTime);
            AppendAttribute(backupDocumentDom, pStateElement, TEXT("lastIncrementalBackup"), dateTime); 
        }
        else
        {
            OutputWriter::WriteLine(TEXT("No last incremental backup time was present.")); 
        }

        OutputWriter::WriteLine(TEXT("Retrieving reference to backup document element.")); 
        CComPtr<IXMLDOMElement> pBackupDocumentElement; 
        CHECK_HRESULT(backupDocumentDom->get_documentElement(&pBackupDocumentElement)); 

        CComPtr<IXMLDOMNode> pThrowawayNode; 
        OutputWriter::WriteLine(TEXT("Removing backup document element.")); 
        CHECK_HRESULT(backupDocumentDom->removeChild(pBackupDocumentElement, &pThrowawayNode)); 
        pThrowawayNode.Release(); 

        OutputWriter::WriteLine(TEXT("Adding backup document element as child of state element.")); 
        CHECK_HRESULT(pStateElement->appendChild(pBackupDocumentElement, &pThrowawayNode)); 
        pThrowawayNode.Release(); 

        OutputWriter::WriteLine(TEXT("Adding state element as document element.")); 
        CHECK_HRESULT(backupDocumentDom->appendChild(pStateElement, &pThrowawayNode)); 
        pThrowawayNode.Release(); 

        OutputWriter::WriteLine(TEXT("Saving backup document.")); 
        CHECK_HRESULT(backupDocumentDom->save(CComVariant(path))); 

        OutputWriter::WriteLine(TEXT("Successfully wrote state file"), VERBOSITY_THRESHOLD_NORMAL); 

    }

};