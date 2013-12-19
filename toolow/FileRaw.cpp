
#include "File.h"

bool File::Raw::open(const wchar_t *path, File::Access access, String *pErr)
{
	this->close(); // make sure everything was properly cleaned up

	_hFile = ::CreateFile(path,
		GENERIC_READ | (access == Access::READWRITE ? GENERIC_WRITE : 0),
		access == Access::READWRITE ? 0 : FILE_SHARE_READ,
		0, access == Access::READWRITE ? OPEN_ALWAYS : OPEN_EXISTING, 0, 0); // if file doesn't exist, will be created

	if(_hFile == INVALID_HANDLE_VALUE) {
		_hFile = NULL;
		if(pErr) pErr->format(L"CreateFile() failed to open file as %s, error code %d.",
			access == Access::READONLY ? L"read-only" : L"read-write", ::GetLastError());
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

	DWORD r = ::SetFilePointer(_hFile, newSize, NULL, FILE_BEGIN);
	if(r == INVALID_SET_FILE_POINTER) {
		DWORD err = ::GetLastError();
		this->close();
		if(pErr) pErr->format(L"SetFilePointer() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	if(!::SetEndOfFile(_hFile)) {
		DWORD err = ::GetLastError();
		this->close();
		if(pErr) pErr->format(L"SetEndOfFile() failed with offset of %d, error code %d.", newSize, err);
		return false;
	}

	r = ::SetFilePointer(_hFile, 0, NULL, FILE_BEGIN); // rewind
	if(r == INVALID_SET_FILE_POINTER) {
		DWORD err = ::GetLastError();
		this->close();
		if(pErr) pErr->format(L"SetFilePointer() failed to rewind the file, error code %d.", err);
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
	if(!::ReadFile(_hFile, &(*pBuf)[0], pBuf->size(), &bytesRead, NULL)) {
		if(pErr) pErr->format(L"ReadFile() failed to read %d bytes.", pBuf->size());
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
	if(!::WriteFile(_hFile, pData, sz, &dwWritten, NULL)) {
		if(pErr) pErr->format(L"WriteFile() failed to write %d bytes.", sz);
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

	if(::SetFilePointer(_hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
		if(pErr) pErr->format(L"SetFilePointer() faile, error code %d.", ::GetLastError());
		return false;
	}
	return true;
}