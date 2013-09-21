
#include "File.h"

bool File::open(const wchar_t *path, File::Access access, String *pErr)
{
	close(); // make sure everything was properly cleaned up

	_hFile = CreateFile(path,
		GENERIC_READ | (access == READWRITE ? GENERIC_WRITE : 0),
		access == READWRITE ? 0 : FILE_SHARE_READ,
		0, access == READWRITE ? OPEN_ALWAYS : OPEN_EXISTING, 0, 0); // if file doesn't exist, will be created

	if(_hFile == INVALID_HANDLE_VALUE) {
		_hFile = 0;
		if(pErr) pErr->fmt(L"CreateFile() failed to open file as %s, error code %d.",
			access == READONLY ? L"read-only" : L"read-write", GetLastError());
		return false;
	}
	
	if(pErr) *pErr = L"";
	return true;
}

bool File::setNewSize(int newSize, String *pErr)
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

bool File::getContent(Array<BYTE> *pBuf, String *pErr)
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

bool File::write(const BYTE *pData, int sz, String *pErr)
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

bool File::rewind(String *pErr)
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