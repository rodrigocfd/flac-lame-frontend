
#include "File.h"

Date File::LastModified(const wchar_t *path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad = { 0 };
	GetFileAttributesEx(path, GetFileExInfoStandard, &fad);
	return Date(&fad.ftLastWriteTime); // already converted to current timezone
}

Date File::Created(const wchar_t *path)
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
	int newLen = WideCharToMultiByte(CP_UTF8, 0, data, dataLen, 0, 0, 0, 0);
	Array<BYTE> outBuf(newLen + (isUtf8 ? 3 : 0));
	if(isUtf8)
		memcpy(&outBuf[0], "\xEF\xBB\xBF", 3); // write UTF-8 BOM
	WideCharToMultiByte(CP_UTF8, 0, data, dataLen, (char*)&outBuf[isUtf8 ? 3 : 0], newLen, 0, 0);
	if(!fout.write(&outBuf, pErr)) // one single write() to all data, better performance
		return false;
	
	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::open(const wchar_t *path, File::Access::Type access, String *pErr)
{
	close(); // make sure everything was properly cleaned up

	_hFile = CreateFile(path,
		GENERIC_READ | (access == Access::READWRITE ? GENERIC_WRITE : 0),
		access == Access::READWRITE ? 0 : FILE_SHARE_READ,
		0, access == Access::READWRITE ? OPEN_ALWAYS : OPEN_EXISTING, 0, 0); // if file doesn't exist, will be created

	if(_hFile == INVALID_HANDLE_VALUE) {
		_hFile = 0;
		if(pErr) pErr->fmt(L"CreateFile() failed to open file as %s, error code %d.",
			access == Access::READONLY ? L"read-only" : L"read-write", GetLastError());
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

	DWORD r = SetFilePointer(_hFile, newSize, 0, FILE_BEGIN);
	if(r == INVALID_SET_FILE_POINTER) {
		this->close();
		if(pErr) pErr->fmt(L"SetFilePointer() failed with offset of %d, error code %d.",
			newSize, GetLastError());
		return false;
	}

	if(!SetEndOfFile(_hFile)) {
		this->close();
		if(pErr) pErr->fmt(L"SetEndOfFile() failed with offset of %d, error code %d.",
			newSize, GetLastError());
		return false;
	}

	r = SetFilePointer(_hFile, 0, 0, FILE_BEGIN); // rewind
	if(r == INVALID_SET_FILE_POINTER) {
		this->close();
		if(pErr) pErr->fmt(L"SetFilePointer() failed to rewind the file, error code %d.",
			GetLastError());
		return false;
	}

	if(pErr) *pErr = L"";
	return true;
}

bool File::Raw::getContent(Array<BYTE> *pBuf, String *pErr)
{
	if(!_hFile) {
		if(pErr) *pErr = L"File has not been opened.";
		return false;
	}

	pBuf->realloc(this->size());
	DWORD bytesRead = 0;
	if(!ReadFile(_hFile, &(*pBuf)[0], pBuf->size(), &bytesRead, 0)) {
		if(pErr) pErr->fmt(L"ReadFile() failed to read %d bytes.", pBuf->size());
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
	if(!WriteFile(_hFile, pData, sz, &dwWritten, 0)) {
		if(pErr) pErr->fmt(L"WriteFile() failed to write %d bytes.", sz);
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

	if(SetFilePointer(_hFile, 0, 0, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if(pErr) pErr->fmt(L"SetFilePointer() faile, error code %d.", GetLastError());
		return false;
	}
	return true;
}