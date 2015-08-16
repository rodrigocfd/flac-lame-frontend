/*!
 * @file
 * @brief Automation for file handling.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "file.h"
#include "str.h"
#include <algorithm>
#include <direct.h> // _wmkdir()
#include <Shlobj.h>
#include <MsXml2.h>
#pragma comment(lib, "msxml2.lib")
using namespace wolf;
using namespace wolf::file;
using namespace wolf::res;
using std::unordered_map;
using std::vector;
using std::wstring;

bool file::Delete(const wchar_t *path, wstring *pErr)
{
	if (IsDir(path)) {
		// http://stackoverflow.com/questions/1468774/why-am-i-having-problems-recursively-deleting-directories
		wchar_t szDir[MAX_PATH + 1] = { 0 }; // +1 for the double null terminate
		lstrcpy(szDir, path);

		SHFILEOPSTRUCTW fos = { 0 };
		fos.wFunc = FO_DELETE;
		fos.pFrom = szDir;
		fos.fFlags = FOF_NO_UI;

		if (!SHFileOperation(&fos)) {
			if (pErr) *pErr = L"SHFileOperation() failed to recursively delete directory.";
			return false;
		}
	} else {
		if (!DeleteFile(path)) {
			if (pErr) *pErr = str::Sprintf(L"DeleteFile() failed, error code %d.", GetLastError());
			return false;
		}
	}
	if (pErr) pErr->clear();
	return true;
}

bool file::CreateDir(const wchar_t *path)
{
	return _wmkdir(path) == 0;
}

Date file::DateLastModified(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(fad.ftLastWriteTime); // already converted to current timezone
}

Date file::DateCreated(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(fad.ftCreationTime); // already converted to current timezone
}

bool file::WriteUtf8(const wchar_t *path, const wchar_t *data, wstring *pErr)
{
	bool isUtf8 = false;
	int dataLen = lstrlen(data);
	for (int i = 0; i < dataLen; ++i) {
		if (data[i] > 127) {
			isUtf8 = true;
			break;
		}
	}

	Raw fout;
	if (!fout.open(path, Access::READWRITE, pErr)) {
		return false;
	}
	if (fout.size() && !fout.setNewSize(0, pErr)) { // if already exists, truncate to empty
		return false;
	}

	// If the text doesn't have any char to make it UTF-8, it'll
	// be simply converted to plain ASCII.
	int newLen = WideCharToMultiByte(CP_UTF8, 0, data, dataLen, nullptr, 0, nullptr, nullptr);
	vector<BYTE> outBuf(newLen + (isUtf8 ? 3 : 0));
	if (isUtf8) memcpy(&outBuf[0], "\xEF\xBB\xBF", 3); // write UTF-8 BOM
	WideCharToMultiByte(CP_UTF8, 0, data, dataLen,
		reinterpret_cast<char*>(&outBuf[isUtf8 ? 3 : 0]), newLen, nullptr, nullptr);
	if (!fout.write(outBuf, pErr)) { // one single write() to all data, better performance
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool file::Unzip(const wchar_t *zip, const wchar_t *destFolder, wstring *pErr)
{
	if (!Exists(zip)) {
		if (pErr) *pErr = str::Sprintf(L"File doesn't exist: \"%s\".", zip);
		return false;
	}
	if (!Exists(destFolder)) {
		if (pErr) *pErr = str::Sprintf(L"Output directory doesn't exist: \"%s\".", destFolder);
		return false;
	}

	// http://social.msdn.microsoft.com/Forums/vstudio/en-US/45668d18-2840-4887-87e1-4085201f4103/visual-c-to-unzip-a-zip-file-to-a-specific-directory
	CoInitialize(nullptr);

	IShellDispatch *pISD = nullptr;
	if (FAILED( CoCreateInstance(CLSID_Shell, nullptr, CLSCTX_INPROC_SERVER,
		IID_IShellDispatch, reinterpret_cast<void**>(&pISD)) ))
	{
		if (pErr) *pErr = L"CoCreateInstance failed on IID_IShellDispatch.";
		return false;
	}

	BSTR bstrZipFile = SysAllocString(zip);
	VARIANT inZipFile = { 0 };
	inZipFile.vt = VT_BSTR;
	inZipFile.bstrVal = bstrZipFile;

	Folder *pZippedFile = nullptr;
	pISD->NameSpace(inZipFile, &pZippedFile);
	if (!pZippedFile) {
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"IShellDispatch::NameSpace() failed on zip file name.";
		return false;
	}

	BSTR bstrFolder = SysAllocString(destFolder);
	VARIANT outFolder = { 0 };
	outFolder.vt = VT_BSTR;
	outFolder.bstrVal = bstrFolder;

	Folder *pDestination = nullptr;
	pISD->NameSpace(outFolder, &pDestination);
	if (!pDestination) {
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"IShellDispatch::NameSpace() failed on directory name.";
		return false;
	}

	FolderItems *pFilesInside = nullptr;
	pZippedFile->Items(&pFilesInside);
	if (!pFilesInside) {
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"Folder::Items() failed.";
		return false;
	}

	long FilesCount = 0;
	pFilesInside->get_Count(&FilesCount);
	if (FilesCount < 1) {
		pFilesInside->Release();
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD->Release();
		CoUninitialize();
		if (pErr) *pErr = L"FolderItems::get_Count() failed.";
		return false;
	}

	IDispatch *pItem = nullptr;
	pFilesInside->QueryInterface(IID_IDispatch, reinterpret_cast<void**>(&pItem));

	VARIANT item = { 0 };
	item.vt = VT_DISPATCH;
	item.pdispVal = pItem;

	VARIANT options = { 0 };
	options.vt = VT_I4;
	options.lVal = 1024 | 512 | 16 | 4; // http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

	bool okay = SUCCEEDED(pDestination->CopyHere(item, options));

	pItem->Release();
	pFilesInside->Release();
	pDestination->Release();
	SysFreeString(bstrFolder);
	pZippedFile->Release();
	SysFreeString(bstrZipFile);
	pISD->Release();
	CoUninitialize();

	if (!okay) {
		if (pErr) *pErr = L"Folder::CopyHere() failed.";
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

int file::IndexOfBin(const BYTE *pData, size_t dataLen, const wchar_t *what, bool asWideChar)
{
	// Returns the position of a string within a binary data block, if present.

	size_t whatlen = lstrlen(what);
	size_t pWhatSz = whatlen * (asWideChar ? 2 : 1);
	BYTE *pWhat = static_cast<BYTE*>(_alloca(pWhatSz * sizeof(BYTE)));
	if (asWideChar) {
		memcpy(pWhat, what, whatlen * sizeof(wchar_t)); // simply copy the wide string, each char+zero
	} else {
		for (size_t i = 0; i < whatlen; ++i) {
			pWhat[i] = LOBYTE(what[i]); // raw conversion from wchar_t to char
		}
	}

	for (size_t i = 0; i < dataLen; ++i) {
		if (!memcmp(pData + i, pWhat, pWhatSz * sizeof(BYTE))) {
			return static_cast<int>(i);
		}
	}
	return -1; // not found
}


void path::ChangeExtension(wstring& sPath, const wchar_t *extWithoutDot)
{
	sPath.resize(str::Find(str::Sens::YES, sPath, L'.') + 1); // truncate after the dot
	sPath.append(extWithoutDot);
}

void path::TrimBackslash(wstring& sPath)
{
	if (!sPath.empty() && sPath.back() == L'\\') {
		sPath.resize(sPath.length() - 1);
	}
}

wstring path::GetPath(const wchar_t *sPath)
{
	wstring ret = sPath;
	ret.resize(str::FindRev(str::Sens::YES, ret, L'\\')); // also remove trailing backslash
	return ret;
}

wstring path::GetFilename(const wstring& sPath)
{
	wstring ret = sPath;
	ret.erase(0, str::FindRev(str::Sens::YES, ret, L'\\') + 1);
	return ret;
}


void Raw::close()
{
	if (_hFile) {
		CloseHandle(_hFile);
		_hFile = nullptr;
		_access = Access::READONLY;
	}
}

bool Raw::open(const wchar_t *path, Access access, wstring *pErr)
{
	this->close(); // make sure everything was properly cleaned up

	_hFile = CreateFile(path,
		GENERIC_READ | (access == Access::READWRITE ? GENERIC_WRITE : 0),
		(access == Access::READWRITE) ? 0 : FILE_SHARE_READ, nullptr,
		(access == Access::READWRITE) ? OPEN_ALWAYS : OPEN_EXISTING,
		0, nullptr); // if file doesn't exist, will be created

	if (_hFile == INVALID_HANDLE_VALUE) {
		_hFile = nullptr;
		if (pErr) *pErr = str::Sprintf(L"CreateFile() failed to open file as %s, error code %d.",
			(access == Access::READONLY) ? L"read-only" : L"read-write", GetLastError());
		return false;
	}

	_access = access; // keep for future checks
	if (pErr) pErr->clear();
	return true;
}

bool Raw::setNewSize(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// Size zero will truncate the file.

	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == Access::READONLY) {
		if (pErr) *pErr = L"File is opened for read-only access.";
		return false;
	}

	DWORD r = SetFilePointer(_hFile, static_cast<LONG>(newSize), nullptr, FILE_BEGIN);
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"SetFilePointer() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	if (!SetEndOfFile(_hFile)) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"SetEndOfFile() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	r = SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN); // rewind
	if (r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"SetFilePointer() failed to rewind the file, error code %d.", err);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool Raw::getContent(vector<BYTE>& buf, wstring *pErr) const
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	buf.resize(this->size());
	DWORD bytesRead = 0;
	if (!ReadFile(_hFile, &buf[0], static_cast<DWORD>(buf.size()), &bytesRead, nullptr)) {
		if (pErr) *pErr = str::Sprintf(L"ReadFile() failed to read %d bytes.", buf.size());
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool Raw::write(const BYTE *pData, size_t sz, wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (_access == Access::READONLY) {
		if (pErr) *pErr = L"File is opened for read-only access.";
		return false;
	}

	// File boundary will be expanded if needed.
	// Internal file pointer will move forward.
	DWORD dwWritten = 0;
	if (!WriteFile(_hFile, pData, static_cast<DWORD>(sz), &dwWritten, nullptr)) {
		if (pErr) *pErr = str::Sprintf(L"WriteFile() failed to write %d bytes.", sz);
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool Raw::rewind(wstring *pErr)
{
	if (!_hFile) {
		if (pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if (SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if (pErr) *pErr = str::Sprintf(L"SetFilePointer() failed, error code %d.", GetLastError());
		return false;
	}
	return true;
}


void Mapped::close()
{
	if (_pMem) {
		UnmapViewOfFile(_pMem);
		_pMem = nullptr;
	}
	if (_hMap) {
		CloseHandle(_hMap);
		_hMap = nullptr;
	}
	_file.close();
	_size = 0;
}

bool Mapped::open(const wchar_t *path, Access access, wstring *pErr)
{
	this->close(); // make sure everything was properly cleaned up

	// Open file.
	if (!_file.open(path, access, pErr)) {
		this->close();
		return false;
	}

	// Mapping into memory.
	_hMap = CreateFileMapping(_file.hFile(), nullptr,
		(access == Access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if (!_hMap) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"CreateFileMapping() failed to create file mapping, error code %d.", err);
		return false;
	}

	// Get pointer to data block.
	_pMem = MapViewOfFile(_hMap,
		(access == Access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if (!_pMem) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"MapViewOfFile() failed to map view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep file size
	if (pErr) pErr->clear();
	return true;
}

bool Mapped::setNewSize(size_t newSize, wstring *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if (!_hMap || !_pMem || !_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	UnmapViewOfFile(_pMem);
	CloseHandle(_hMap);

	// Truncate/expand file.
	if (!_file.setNewSize(newSize, pErr)) {
		this->close();
		return false;
	}

	// Remap into memory.
	if (!( _hMap = CreateFileMapping(_file.hFile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"CreateFileMapping() failed to recreate file mapping, error code %d.", err);
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if (!( _pMem = MapViewOfFile(_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		DWORD err = GetLastError();
		this->close();
		if (pErr) *pErr = str::Sprintf(L"MapViewOfFile() failed to remap view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep new file size
	if (pErr) pErr->clear();
	return true;
}

bool Mapped::getContent(vector<BYTE>& buf, int offset, int numBytes, wstring *pErr) const
{
	if (!_hMap || !_pMem || !_file.hFile()) {
		if (pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if (offset >= static_cast<int>(_size)) {
		if (pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if (numBytes == -1 || offset + numBytes > static_cast<int>(_size)) {
		numBytes = static_cast<int>(_size) - offset; // avoid reading beyond EOF
	}

	buf.resize(numBytes);
	memcpy(&buf[0], this->pMem(), numBytes * sizeof(BYTE));

	if (pErr) pErr->clear();
	return true;
}

bool Mapped::getContent(wstring& buf, int offset, int numChars, wstring *pErr) const
{
	vector<BYTE> byteBuf;
	if (!this->getContent(byteBuf, offset, numChars, pErr)) {
		return false;
	}

	buf.resize(byteBuf.size());
	for (size_t i = 0; i < byteBuf.size(); ++i) {
		buf[i] = static_cast<wchar_t>(byteBuf[i]); // raw conversion
	}

	if (pErr) pErr->clear();
	return true;
}


bool Text::load(const wchar_t *path, wstring *pErr)
{
	Mapped fm;
	if (!fm.open(path, Access::READONLY, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return this->load(fm);
}

bool Text::load(const Mapped& fm)
{
	BYTE *pMem = fm.pMem(); // file reading is made upon a memory-mapped file, load all into wstring
	BYTE *pPast = fm.pPastMem();

	if ((pPast - pMem >= 3) && !memcmp(pMem, "\xEF\xBB\xBF", 3)) // UTF-8
	{
		pMem += 3; // skip BOM
		_text = str::ParseUtf8(pMem, static_cast<int>(pPast - pMem)); // the whole file is loaded as wchar_t
	}
	else if ((pPast - pMem >= 4) && !memcmp(pMem, "\x00\x00\xFE\xFF", 4)) // UTF-32 BE
	{
		pMem += 4;
		return false;
		//...
	}
	else if ((pPast - pMem >= 4) && !memcmp(pMem, "\xFF\xFE\x00\x00", 4)) // UTF-32 LE
	{
		pMem += 4;
		return false;
		//...
	}
	else if ((pPast - pMem >= 2) && !memcmp(pMem, "\xFE\xFF", 2)) // UTF-16 BE
	{
		pMem += 2;
		_text.resize(static_cast<int>(pPast - pMem) / 2);
		for (int i = 0; i < static_cast<int>(pPast - pMem); i += 2) {
			_text[i / 2] = static_cast<wchar_t>(MAKEWORD(*(pMem + i + 1), *(pMem + i)));
		}
	}
	else if ((pPast - pMem >= 2) && !memcmp(pMem, "\xFF\xFE", 2)) // UTF-16 LE
	{
		pMem += 2;
		_text.resize(static_cast<int>(pPast - pMem) / 2);
		for (int i = 0; i < static_cast<int>(pPast - pMem); i += 2) {
			_text[i / 2] = static_cast<wchar_t>(MAKEWORD(*(pMem + i), *(pMem + i + 1)));
		}
	}
	else // ASCII
	{
		int len = static_cast<int>(pPast - pMem);
		_text.resize(len);
		for (int i = 0; i < len; ++i) {
			_text[i] = static_cast<wchar_t>(*(pMem + i)); // brute-force char to wchar_t
		}
	}
	
	str::TrimNulls(_text);
	this->rewind(); // our seeking pointer to be consumed by nextLine()
	return true;
}

bool Text::nextLine(wstring& buf)
{
	if (!*_p) return false; // runner pointer inside our _text String data block

	if (_idxLine > -1) { // not 1st line; avoid a 1st blank like to be skipped
		if ( (*_p == L'\r' && *(_p + 1) == L'\n') || // CRLF || LFCR
			(*_p == L'\n' && *(_p + 1) == L'\r') )
		{
			_p += 2;
		} else if (*_p == L'\r' || *_p == L'\n') {
			++_p; // CR || LF
		}
	}
	++_idxLine;

	wchar_t *pRun = _p;
	while (*pRun && *pRun != L'\r' && *pRun != '\n') ++pRun;
	buf.clear();
	buf.insert(0, _p, static_cast<size_t>(pRun - _p)); // line won't have CR nor LF at end

	_p = pRun; // consume
	return true;
}


bool Ini::load(wstring *pErr)
{
	if (_path.empty()) {
		if (pErr) *pErr = L"INI path not set.";
		return false;
	}

	Text fin;
	if (!fin.load(_path, pErr)) {
		if (pErr) pErr->insert(0, L"INI file failed to load.\n");
		return false;
	}

	this->sections.clear();
	this->sections.reserve(this->_countSections(&fin));

	wstring line, curSection, name, valstr; // name/val declared here to save reallocs
	while (fin.nextLine(line)) {
		if (line[0] == L'[' && line.back() == L']') { // begin of section found
			curSection.clear();
			curSection.insert(0, &line[1], line.length() - 2);
			this->sections.emplace(curSection, unordered_map<wstring, wstring>()); // new section added
			continue;
		}
		if (!this->sections.empty() && !line.empty()) { // keys will be read only if within a section
			int idxEq = str::Find(str::Sens::YES, line, L'=');
			if (idxEq > -1) {
				name.clear();
				name.insert(0, &line[0], idxEq);
				str::Trim(name);
				
				valstr.clear();
				valstr.insert(0, &line[idxEq + 1], line.length() - (idxEq + 1));
				str::Trim(valstr);

				this->sections[curSection].emplace(name, valstr);
			}
		}
	}

	if (pErr) pErr->clear();
	return true;
}

bool Ini::serialize(wstring *pErr) const
{
	if (_path.empty()) {
		if (pErr) *pErr = L"INI path not set.";
		return false;
	}

	wstring out;
	out.reserve(100);
	for (auto& section : this->sections) {
		out.append(L"[").append(section.first).append(L"]\r\n");
		for (auto& entry : section.second) {
			out.append(entry.first).append(L"=").append(entry.second).append(L"\r\n");
		}
		out.append(L"\r\n");
	}

	if (!WriteUtf8(_path.c_str(), out.c_str(), pErr)) {
		if (pErr) pErr->insert(0, L"INI file serialization failed.\n");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

int Ini::_countSections(Text *fin) const
{
	int count = 0;
	wstring line;
	fin->rewind();
	while (fin->nextLine(line)) {
		if (line[0] == L'[' && line.back() == L']') {
			++count;
		}
	}
	fin->rewind();
	return count;
}


static void _ReadAttrs(IXMLDOMNode *xmlnode, unordered_map<wstring, wstring>& attrbuf)
{
	// Read attribute collection.
	IXMLDOMNamedNodeMap *attrs = nullptr;
	xmlnode->get_attributes(&attrs);
	
	long attrCount = 0;
	attrs->get_length(&attrCount);
	attrbuf.clear();
	attrbuf.reserve(attrCount);

	for (long i = 0; i < attrCount; ++i) {
		IXMLDOMNode *attr = nullptr;
		attrs->get_item(i, &attr);

		DOMNodeType type = NODE_INVALID;
		attr->get_nodeType(&type);
		if (type == NODE_ATTRIBUTE) {
			BSTR bstr = nullptr;
			attr->get_nodeName(&bstr); // get attribute name

			VARIANT var = { 0 };
			attr->get_nodeValue(&var); // get attribute value
			
			attrbuf.emplace(static_cast<wchar_t*>(bstr), static_cast<wchar_t*>(var.bstrVal)); // add hash entry
			SysFreeString(bstr);
			VariantClear(&var);
		}
		attr->Release();
	}
	attrs->Release();
}

static int _CountChildNodes(IXMLDOMNodeList *nodeList)
{
	int childCount = 0;
	long totalCount = 0;
	nodeList->get_length(&totalCount); // includes text and actual element nodes
	
	for (long i = 0; i < totalCount; ++i) {
		IXMLDOMNode *child = nullptr;
		nodeList->get_item(i, &child);

		DOMNodeType type = NODE_INVALID;
		child->get_nodeType(&type);
		if (type == NODE_ELEMENT) ++childCount;

		child->Release();
	}
	return childCount;
}

static void _BuildNode(IXMLDOMNode *xmlnode, Xml::Node& nodebuf)
{
	// Get node name.
	BSTR bstr = nullptr;
	xmlnode->get_nodeName(&bstr);
	nodebuf.name = static_cast<wchar_t*>(bstr);
	SysFreeString(bstr);

	// Parse attributes of node, if any.
	_ReadAttrs(xmlnode, nodebuf.attrs);

	// Process children, if any.
	VARIANT_BOOL vb = FALSE;
	xmlnode->hasChildNodes(&vb);
	if (vb) {
		IXMLDOMNodeList *nodeList = nullptr;
		xmlnode->get_childNodes(&nodeList);
		nodebuf.children.resize(_CountChildNodes(nodeList));

		int childCount = 0;
		long totalCount = 0;
		nodeList->get_length(&totalCount);

		for (long i = 0; i < totalCount; ++i) {
			IXMLDOMNode *child = nullptr;
			nodeList->get_item(i, &child);

			// Node can be text or an actual child node.
			DOMNodeType type = NODE_INVALID;
			child->get_nodeType(&type);
			if (type == NODE_TEXT) {
				xmlnode->get_text(&bstr);
				nodebuf.value.append(static_cast<wchar_t*>(bstr));
				SysFreeString(bstr);
			} else if (type == NODE_ELEMENT) {
				_BuildNode(child, nodebuf.children[childCount++]); // recursively
			} else {
				// (L"Unhandled node type: %d.\n", type);
			}
			child->Release();
		}
		nodeList->Release();
	} else {
		// Assumes that only a leaf node can have text.
		xmlnode->get_text(&bstr);
		nodebuf.value = static_cast<wchar_t*>(bstr);
		SysFreeString(bstr);
	}
}

vector<Xml::Node*> Xml::Node::getChildrenByName(const wchar_t *elemName)
{
	int howMany = 0;
	size_t firstIndex = -1, lastIndex = -1;
	for (size_t i = 0; i < this->children.size(); ++i) {
		if (str::Equals(str::Sens::NO, this->children[i].name, elemName)) { // case-insensitive match
			++howMany;
			if (firstIndex == -1) firstIndex = i;
			lastIndex = i;
		}
	}

	vector<Node*> nodeBuf;
	nodeBuf.reserve(howMany); // alloc return array

	howMany = 0;
	for (size_t i = firstIndex; i <= lastIndex; ++i) {
		if (str::Equals(str::Sens::NO, this->children[i].name, elemName)) {
			nodeBuf.emplace_back(&this->children[i]);
		}
	}
	return nodeBuf;
}

Xml::Node* Xml::Node::firstChildByName(const wchar_t *elemName)
{
	for (Node& node : this->children) {
		if (str::Equals(str::Sens::NO, node.name, elemName)) { // case-insensitive match
			return &node;
		}
	}
	return nullptr; // not found
}

bool Xml::parse(const wchar_t *str)
{
	CoInitialize(nullptr); // http://stackoverflow.com/questions/7824383/double-calls-to-coinitialize
	
	// Create COM object for XML document.
	IXMLDOMDocument2 *doc = nullptr;
	CoCreateInstance(CLSID_DOMDocument30, nullptr, CLSCTX_INPROC_SERVER,
		IID_IXMLDOMDocument, reinterpret_cast<void**>(&doc));
	doc->put_async(FALSE);

	// Parse the XML string.
	VARIANT_BOOL vb = FALSE;
	doc->loadXML(static_cast<BSTR>(const_cast<wchar_t*>(str)), &vb);

	// Get document element and root node from XML.
	IXMLDOMElement *docElem = nullptr;
	doc->get_documentElement(&docElem);

	IXMLDOMNode *rootNode = nullptr;
	docElem->QueryInterface(IID_IXMLDOMNode, reinterpret_cast<void**>(&rootNode));
	_BuildNode(rootNode, this->root); // recursive

	rootNode->Release(); // must be released before CoUninitialize
	docElem->Release();
	doc->Release();
	CoUninitialize();
	return true;
}


Listing::Listing(const wchar_t *pattern)
	: _hFind(nullptr)
{
	SecureZeroMemory(&_wfd, sizeof(_wfd));
	_pattern = pattern; // example of pattern: L"*.mp3"
}

Listing::Listing(const wchar_t *path, const wchar_t *pattern)
	: _hFind(nullptr)
{
	SecureZeroMemory(&_wfd, sizeof(_wfd));
	_pattern = path;
	if (path[lstrlen(path) - 1] != L'\\') _pattern.append(L"\\");
	_pattern.append(pattern);
}

Listing::~Listing()
{
	if (_hFind && _hFind != INVALID_HANDLE_VALUE) {
		FindClose(_hFind);
		_hFind = nullptr;
	}
}

bool Listing::next(wstring& buf)
{
	if (!_hFind) { // first call to method
		if ((_hFind = FindFirstFile(_pattern.c_str(), &_wfd)) == INVALID_HANDLE_VALUE) { // init iteration
			_hFind = nullptr;
			return false; // no files found at all
		}
	} else { // subsequent calls
		if (!FindNextFile(_hFind, &_wfd)) {
			FindClose(_hFind);
			_hFind = nullptr;
			return false; // search finished
		}
	}

	if (str::Find(str::Sens::YES, _pattern, L'\\') != -1) { // user pattern contains an absolute path
		buf = path::GetPath(_pattern);
		buf.append(L"\\").append(_wfd.cFileName);
	} else {
		buf = _wfd.cFileName;
	}

	return true; // more to come, call again
}

static vector<wstring> _ListingGetAll(Listing& listing)
{
	vector<wstring> ret;
	wstring f;
	while (listing.next(f)) {
		ret.emplace_back(f);
	}
	std::sort(ret.begin(), ret.end(), [](const wstring& a, const wstring& b)->bool {
		return str::LexCmp(str::Sens::NO, a, b) < 0;
	});
	return ret;
}

vector<wstring> Listing::GetAll(const wchar_t *pattern)
{
	return _ListingGetAll(Listing(pattern));
}

vector<wstring> Listing::GetAll(const wchar_t *path, const wchar_t *pattern)
{
	return _ListingGetAll(Listing(path, pattern));
}