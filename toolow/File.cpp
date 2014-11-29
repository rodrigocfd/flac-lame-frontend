//
// File handling.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "File.h"
#include "Ptr.h"
#include <direct.h> // _wmkdir()
#include <Shlobj.h>

bool File::Delete(const wchar_t *path, String *pErr)
{
	if(IsDir(path)) {
		// http://stackoverflow.com/questions/1468774/why-am-i-having-problems-recursively-deleting-directories
		wchar_t szDir[MAX_PATH + 1]; // +1 for the double null terminate
		lstrcpy(szDir, path);
		szDir[lstrlen(szDir) + 1] = L'\0'; // double null terminate

		SHFILEOPSTRUCTW fos = { 0 };
		fos.wFunc = FO_DELETE;
		fos.pFrom = szDir;
		fos.fFlags = FOF_NO_UI;

		if(!SHFileOperation(&fos)) {
			if(pErr) *pErr = L"SHFileOperation() failed to recursively delete directory.";
			return false;
		}
	} else {
		if(!DeleteFile(path)) {
			if(pErr) *pErr = String::Fmt(L"DeleteFile() failed, error code %d.", GetLastError());
			return false;
		}
	}
	if(pErr) *pErr = L"";
	return true;
}

bool File::CreateDir(const wchar_t *path)
{
	return _wmkdir(path) == 0;
}

Date File::DateLastModified(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(&fad.ftLastWriteTime); // already converted to current timezone
}

Date File::DateCreated(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(&fad.ftCreationTime); // already converted to current timezone
}

bool File::WriteUtf8(const wchar_t *path, const wchar_t *data, String *pErr)
{
	bool isUtf8 = false;
	int dataLen = lstrlen(data);
	for(int i = 0; i < dataLen; ++i) {
		if(data[i] > 127) {
			isUtf8 = true;
			break;
		}
	}
	
	File::Raw fout;
	if(!fout.open(path, Access::READWRITE, pErr))
		return false;
	if(fout.size() && !fout.setNewSize(0, pErr)) // if already exists, truncate to empty
		return false;
	
	// If the text doesn't have any char to make it UTF-8, it'll
	// be simply converted to plain ASCII.
	int newLen = WideCharToMultiByte(CP_UTF8, 0, data, dataLen, nullptr, 0, nullptr, nullptr);
	Array<BYTE> outBuf(newLen + (isUtf8 ? 3 : 0));
	if(isUtf8)
		memcpy(&outBuf[0], "\xEF\xBB\xBF", 3); // write UTF-8 BOM
	WideCharToMultiByte(CP_UTF8, 0, data, dataLen, (char*)&outBuf[isUtf8 ? 3 : 0], newLen, nullptr, nullptr);
	if(!fout.write(outBuf, pErr)) // one single write() to all data, better performance
		return false;
	
	if(pErr) *pErr = L"";
	return true;
}

bool File::Unzip(const wchar_t *zip, const wchar_t *destFolder, String *pErr)
{
	if(!Exists(zip)) {
		if(pErr) *pErr = String::Fmt(L"File doesn't exist: \"%s\".", zip);
		return false;
	}
	if(!Exists(destFolder)) {
		if(pErr) *pErr = String::Fmt(L"Output directory doesn't exist: \"%s\".", destFolder);
		return false;
	}

	// http://social.msdn.microsoft.com/Forums/vstudio/en-US/45668d18-2840-4887-87e1-4085201f4103/visual-c-to-unzip-a-zip-file-to-a-specific-directory
	CoInitialize(nullptr);

	ComPtr<IShellDispatch> pISD;
	if(!pISD.coCreateInstance(CLSID_Shell, IID_IShellDispatch)) {
		if(pErr) *pErr = L"CoCreateInstance failed on IID_IShellDispatch.";
		return false;
	}

	BSTR bstrZipFile = SysAllocString(zip);
	VARIANT InZipFile;
	InZipFile.vt = VT_BSTR;
	InZipFile.bstrVal = bstrZipFile;

	Folder *pZippedFile = nullptr;
	pISD->NameSpace(InZipFile, &pZippedFile);
	if(!pZippedFile) {
		SysFreeString(bstrZipFile);
		pISD.release();
		CoUninitialize();
		if(pErr) *pErr = L"IShellDispatch::NameSpace() failed on zip file name.";
		return false;
	}

	BSTR bstrFolder = SysAllocString(destFolder);
	VARIANT OutFolder;
	OutFolder.vt = VT_BSTR;
	OutFolder.bstrVal = bstrFolder;

	Folder *pDestination = nullptr;
	pISD->NameSpace(OutFolder, &pDestination);
	if(!pDestination) {
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD.release();
		CoUninitialize();
		if(pErr) *pErr = L"IShellDispatch::NameSpace() failed on directory name.";
		return false;
	}
    
	FolderItems *pFilesInside = nullptr;
	pZippedFile->Items(&pFilesInside);
	if(!pFilesInside) {
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD.release();
		CoUninitialize();
		if(pErr) *pErr = L"Folder::Items() failed.";
		return false;
	}

	long FilesCount = 0;
	pFilesInside->get_Count(&FilesCount);
	if(FilesCount < 1) {
		pFilesInside->Release();
		pDestination->Release();
		SysFreeString(bstrFolder);
		pZippedFile->Release();
		SysFreeString(bstrZipFile);
		pISD.release();
		CoUninitialize();
		if(pErr) *pErr = L"FolderItems::get_Count() failed.";
		return false;
	}

	IDispatch *pItem = nullptr;
	pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);

	VARIANT Item;
	Item.vt = VT_DISPATCH;
	Item.pdispVal = pItem;

	VARIANT Options;
	Options.vt = VT_I4;
	Options.lVal = 1024 | 512 | 16 | 4; // http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

	bool okay = SUCCEEDED(pDestination->CopyHere(Item, Options));

	pItem->Release();
	pFilesInside->Release();
	pDestination->Release();
	SysFreeString(bstrFolder);
	pZippedFile->Release();
	SysFreeString(bstrZipFile);
	pISD.release();
	CoUninitialize();

	if(!okay) {
		if(pErr) *pErr = L"Folder::CopyHere() failed.";
		return false;
	}
	if(pErr) *pErr = L"";
	return true;
}

int File::IndexOfBin(const BYTE *pData, int dataLen, const wchar_t *what, bool asWideChar)
{
	// Returns the position of a string within a binary data block, if present.

	int whatlen = lstrlen(what);
	int pWhatSz = whatlen * (asWideChar ? 2 : 1);
	BYTE *pWhat = (BYTE*)_alloca(pWhatSz * sizeof(BYTE));
	if(asWideChar) {
		memcpy(pWhat, what, whatlen * sizeof(wchar_t)); // simply copy the wide string, each char+zero
	} else {
		for(int i = 0; i < whatlen; ++i)
			pWhat[i] = LOBYTE(what[i]); // raw conversion from wchar_t to char
	}

	for(int i = 0; i < dataLen; ++i)
		if(!memcmp(pData + i, pWhat, pWhatSz * sizeof(BYTE)))
			return i;

	return -1; // not found
}


bool File::Raw::open(const wchar_t *path, File::Access access, String *pErr)
{
	this->close(); // make sure everything was properly cleaned up

	_hFile = CreateFile(path,
		GENERIC_READ | (access == Access::READWRITE ? GENERIC_WRITE : 0),
		(access == Access::READWRITE) ? 0 : FILE_SHARE_READ, nullptr,
		(access == Access::READWRITE) ? OPEN_ALWAYS : OPEN_EXISTING,
		0, nullptr); // if file doesn't exist, will be created

	if(_hFile == INVALID_HANDLE_VALUE) {
		_hFile = nullptr;
		if(pErr) *pErr = String::Fmt(L"CreateFile() failed to open file as %s, error code %d.",
			(access == Access::READONLY) ? L"read-only" : L"read-write", GetLastError());
		return false;
	}
	
	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::setNewSize(int newSize, String *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.
	// Size zero will truncate the file.

	if(!_hFile) {
		if(pErr) *pErr = L"File has not been opened.";
		return false;
	}

	DWORD r = SetFilePointer(_hFile, newSize, nullptr, FILE_BEGIN);
	if(r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"SetFilePointer() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	if(!SetEndOfFile(_hFile)) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"SetEndOfFile() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	r = SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN); // rewind
	if(r == INVALID_SET_FILE_POINTER) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"SetFilePointer() failed to rewind the file, error code %d.", err);
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::getContent(Array<BYTE> *pBuf, String *pErr) const
{
	if(!_hFile) {
		if(pErr) *pErr = L"File has not been opened.";
		return false;
	}

	pBuf->resize(this->size());
	DWORD bytesRead = 0;
	if(!ReadFile(_hFile, &(*pBuf)[0], pBuf->size(), &bytesRead, nullptr)) {
		if(pErr) *pErr = String::Fmt(L"ReadFile() failed to read %d bytes.", pBuf->size());
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::write(const BYTE *pData, int sz, String *pErr)
{
	if(!_hFile) {
		if(pErr) *pErr = L"File has not been opened.";
		return false;
	}

	// File boundary will be expanded if needed.
	// Internal file pointer will move forward.
	DWORD dwWritten = 0;
	if(!WriteFile(_hFile, pData, sz, &dwWritten, nullptr)) {
		if(pErr) *pErr = String::Fmt(L"WriteFile() failed to write %d bytes.", sz);
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::rewind(String *pErr)
{
	if(!_hFile) {
		if(pErr) *pErr = L"File has not been opened.";
		return false;
	}

	if(SetFilePointer(_hFile, 0, nullptr, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if(pErr) *pErr = String::Fmt(L"SetFilePointer() faile, error code %d.", GetLastError());
		return false;
	}
	return true;
}


void File::Mapped::close()
{
	if(_pMem) { UnmapViewOfFile(_pMem); _pMem = nullptr; }
	if(_hMap) { CloseHandle(_hMap); _hMap = nullptr; }
	_file.close();
	_size = 0;
}

bool File::Mapped::open(const wchar_t *path, File::Access access, String *pErr)
{
	this->close(); // make sure everything was properly cleaned up
	
	// Open file.
	if(!_file.open(path, access, pErr)) {
		this->close();
		return false;
	}

	// Mapping into memory.
	_hMap = CreateFileMapping(_file.hFile(), nullptr,
		(access == Access::READWRITE) ? PAGE_READWRITE : PAGE_READONLY, 0, 0, nullptr);
	if(!_hMap) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"CreateFileMapping() failed to create file mapping, error code %d.", err);
		return false;
	}

	// Get pointer to data block.
	_pMem = MapViewOfFile(_hMap,
		(access == Access::READWRITE) ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
	if(!_pMem) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"MapViewOfFile() failed to map view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep file size
	if(pErr) *pErr = L"";
	return true;
}

bool File::Mapped::setNewSize(int newSize, String *pErr)
{
	// This method will truncate or expand the file, according to the new size.
	// It will probably fail if file was opened as read-only.

	if(!_hMap || !_pMem || !_file.hFile()) {
		if(pErr) *pErr = L"File is not mapped into memory.";
		return false;
	}

	// Unmap file, but keep it open.
	UnmapViewOfFile(_pMem);
	CloseHandle(_hMap);

	// Truncate/expand file.
	if(!_file.setNewSize(newSize, pErr)) {
		this->close();
		return false;
	}

	// Remap into memory.
	if(!( _hMap = CreateFileMapping(_file.hFile(), 0, PAGE_READWRITE, 0, 0, nullptr) )) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"CreateFileMapping() failed to recreate file mapping, error code %d.", err);
		return false;
	}

	// Get new pointer to data block, old one just became invalid!
	if(!( _pMem = MapViewOfFile(_hMap, FILE_MAP_WRITE, 0, 0, 0) )) {
		DWORD err = GetLastError();
		this->close();
		if(pErr) *pErr = String::Fmt(L"MapViewOfFile() failed to remap view of file, error code %d.", err);
		return false;
	}

	_size = _file.size(); // keep new file size
	if(pErr) *pErr = L"";
	return true;
}

bool File::Mapped::getContent(Array<BYTE> *pBuf, int offset, int numBytes, String *pErr) const
{
	if(!_hMap || !_pMem || !_file.hFile()) {
		if(pErr) *pErr = L"File is not mapped into memory.";
		return false;
	} else if(offset >= _size) {
		if(pErr) *pErr = L"Offset is beyond end of file.";
		return false;
	} else if(numBytes == -1 || offset + numBytes > _size) {
		numBytes = _size - offset; // avoid reading beyond EOF
	}
	
	pBuf->resize(numBytes);
	memcpy(&(*pBuf)[0], this->pMem(), numBytes * sizeof(BYTE));
	
	if(pErr) *pErr = L"";
	return true;
}

bool File::Mapped::getContent(String *pBuf, int offset, int numChars, String *pErr) const
{
	Array<BYTE> byteBuf;
	if(!this->getContent(&byteBuf, offset, numChars, pErr))
		return false;

	pBuf->reserve(byteBuf.size());
	for(int i = 0; i < byteBuf.size(); ++i)
		(*pBuf)[i] = (wchar_t)byteBuf[i]; // raw conversion
	
	if(pErr) *pErr = L"";
	return true;
}


bool File::Text::load(const wchar_t *path, String *pErr)
{
	File::Mapped fm;
	if(!fm.open(path, Access::READONLY, pErr))
		return false;
	if(pErr) *pErr = L"";
	return this->load(&fm);
}

bool File::Text::load(const File::Mapped *pfm)
{
	BYTE *pMem = pfm->pMem(); // the file reading is made upon a memory-mapped file
	BYTE *pPast = pfm->pPastMem();
	
	if((pPast - pMem >= 3) && !memcmp(pMem, "\xEF\xBB\xBF", 3)) // UTF-8
	{
		pMem += 3; // skip BOM
		_text = String::ParseUtf8(pMem, (int)(pPast - pMem)); // the whole file is loaded into a String as wchar_t
	}
	else if((pPast - pMem >= 4) && !memcmp(pMem, "\x00\x00\xFE\xFF", 4)) // UTF-32 BE
	{
		pMem += 4;
		return false;
		//...
	}
	else if((pPast - pMem >= 4) && !memcmp(pMem, "\xFF\xFE\x00\x00", 4)) // UTF-32 LE
	{
		pMem += 4;
		return false;
		//...
	}
	else if((pPast - pMem >= 2) && !memcmp(pMem, "\xFE\xFF", 2)) // UTF-16 BE
	{
		pMem += 2;
		_text.reserve((int)(pPast - pMem) / 2);
		for(int i = 0; i < (int)(pPast - pMem); i += 2)
			_text[i / 2] = (wchar_t)MAKEWORD(*(pMem + i + 1), *(pMem + i));
	}
	else if((pPast - pMem >= 2) && !memcmp(pMem, "\xFF\xFE", 2)) // UTF-16 LE
	{
		pMem += 2;
		_text.reserve((int)(pPast - pMem) / 2);
		for(int i = 0; i < (int)(pPast - pMem); i += 2)
			_text[i / 2] = (wchar_t)MAKEWORD(*(pMem + i), *(pMem + i + 1));
	}
	else // ASCII
	{
		int len = (int)(pPast - pMem);
		_text.reserve(len);
		for(int i = 0; i < len; ++i)
			_text[i] = (wchar_t)*(pMem + i); // brute-force char to wchar_t
	}

	this->rewind(); // our seeking pointer to be consumed by nextLine()
	return true;
}

bool File::Text::nextLine(String *pBuf)
{
	if(!*_p) return false; // runner pointer inside our _text String data block
	
	if(_idxLine > -1) { // not 1st line; avoid a 1st blank like to be skipped
		if( (*_p == L'\r' && *(_p + 1) == L'\n') || // CRLF || LFCR
			(*_p == L'\n' && *(_p + 1) == L'\r') ) _p += 2;
		else if(*_p == L'\r' || *_p == L'\n') ++_p; // CR || LF
	}
	++_idxLine;
	
	wchar_t *pRun = _p;
	while(*pRun && *pRun != L'\r' && *pRun != '\n') ++pRun;
	pBuf->reserve((int)(pRun - _p));
	pBuf->copyFrom(_p, (int)(pRun - _p)); // line won't have CR nor LF at end
	
	_p = pRun; // consume
	return true;
}


bool File::Ini::load(String *pErr)
{
	if(_path.isEmpty()) {
		if(pErr) *pErr = L"INI path not set.";
		return false;
	}

	File::Text fin;
	if(!fin.load(_path.str(), pErr)) {
		if(pErr) pErr->insert(0, L"INI file failed to load.\n");
		return false;
	}

	this->sections.removeAll().reserve( this->_countSections(&fin) );

	String line, name, valstr; // name/val declared here to save reallocs
	while(fin.nextLine(&line)) {
		if(line[0] == L'[' && line.endsWithCS(L']')) { // begin section found
			name.copyFrom(line.ptrAt(1), line.len() - 2);
			this->sections[name] = Hash<String>(); // new section is an empty hash
			continue;
		}
		if(this->sections.size() && line.len()) { // keys will be read only if within a section
			int idxEq = line.findCS(L'=');
			if(idxEq > -1) {
				name.copyFrom(line.ptrAt(0), idxEq);
				valstr.copyFrom(line.ptrAt(idxEq + 1), line.len() - (idxEq + 1));

				Hash<String> *pLastSection = &this->sections.at(this->sections.size() - 1)->val;
				(*pLastSection)[name.trim()] = valstr.trim();
			}
		}
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Ini::serialize(String *pErr) const
{
	if(_path.isEmpty()) {
		if(pErr) *pErr = L"INI path not set.";
		return false;
	}

	String out;
	for(int i = 0; i < this->sections.size(); ++i) {
		const Hash<Hash<String>>::Elem *section = this->sections.at(i);
		out.append(L'[').append( section->key.str() ).append(L"]\r\n");

		const Hash<String> *entries = &section->val;
		for(int j = 0; j < entries->size(); ++j) {
			const Hash<String>::Elem *entry = entries->at(j);
			out.append( entry->key.str() ).append(L'=').append( entry->val.str() ).append(L"\r\n");
		}
		
		out.append(L"\r\n");
	}

	if(!File::WriteUtf8(_path.str(), out.str(), pErr)) {
		if(pErr) pErr->insert(0, L"INI file serialization failed.\n");
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

int File::Ini::_countSections(File::Text *fin) const
{
	int count = 0;
	String line;
	fin->rewind();
	while(fin->nextLine(&line))
		if(line[0] == L'[' && line.endsWithCS(L']'))
			++count;
	fin->rewind();
	return count;
}


File::Listing::Listing(const wchar_t *pattern)
	: _hFind(nullptr)
{
	SecureZeroMemory(&_wfd, sizeof(_wfd));
	_pattern = _wcsdup(pattern); // example of pattern: L"*.mp3"
}

File::Listing::Listing(const wchar_t *path, const wchar_t *pattern)
	: _hFind(nullptr)
{
	SecureZeroMemory(&_wfd, sizeof(_wfd));
	bool hasBackslash = path[lstrlen(path) - 1] == L'\\';
	_pattern = (wchar_t*)malloc(sizeof(wchar_t) * (
		lstrlen(path) +
		(hasBackslash ? 0 : 1) +
		lstrlen(pattern) + 1 ));
	lstrcpy(_pattern, path);
	if(!hasBackslash) lstrcat(_pattern, L"\\");
	lstrcat(_pattern, pattern); // assembly path + pattern
}

File::Listing::~Listing()
{
	free(_pattern);
	if(_hFind && _hFind != INVALID_HANDLE_VALUE)
		FindClose(_hFind);
}

wchar_t* File::Listing::next(wchar_t *buf)
{
	if(!_hFind) { // first call to method
		if((_hFind = FindFirstFile(_pattern, &_wfd)) == INVALID_HANDLE_VALUE) { // init iteration
			_hFind = nullptr;
			return nullptr; // no files found at all
		}
	} else { // subsequent calls
		if(!FindNextFile(_hFind, &_wfd)) {
			FindClose(_hFind);
			_hFind = nullptr;
			return nullptr; // search finished
		}
	}

	const wchar_t *pBackslash;
	if(pBackslash = wcsrchr(_pattern, L'\\')) { // search last backslash on user pattern
		int dirnameLen = (int)(pBackslash - _pattern) + 1; // length of directory plus backslash
		lstrcpyn(buf, _pattern, dirnameLen + 1); // number of chars includes the terminating null
		lstrcat(buf, _wfd.cFileName); // filepath + filename
	} else {
		lstrcpy(buf, _wfd.cFileName); // simply copy
	}
	
	return buf; // same passed buffer
}

String* File::Listing::next(String *pBuf)
{
	wchar_t stackbuf[MAX_PATH];
	if(this->next(stackbuf)) {
		*pBuf = stackbuf;
		return pBuf;
	}
	*pBuf = L"";
	return nullptr; // search finished
}