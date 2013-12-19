
#include "File.h"

File::Enum::Enum(const wchar_t *pattern) : _hFind(NULL) {
	::SecureZeroMemory(&_wfd, sizeof(_wfd));
	_pattern = ::_wcsdup(pattern); // example of pattern: L"*.mp3"
}

File::Enum::Enum(const wchar_t *path, const wchar_t *pattern) : _hFind(NULL) {
	::SecureZeroMemory(&_wfd, sizeof(_wfd));
	bool hasBackslash = path[::lstrlen(path) - 1] == L'\\';
	_pattern = (wchar_t*)::malloc(sizeof(wchar_t) * (
		::lstrlen(path) +
		(hasBackslash ? 0 : 1) +
		::lstrlen(pattern) + 1 ));
	::lstrcpy(_pattern, path);
	if(!hasBackslash) ::lstrcat(_pattern, L"\\");
	::lstrcat(_pattern, pattern); // assembly path + pattern
}

File::Enum::~Enum() {
	::free(_pattern);
	if(_hFind && _hFind != INVALID_HANDLE_VALUE)
		::FindClose(_hFind);
}

wchar_t* File::Enum::next(wchar_t *buf) {
	if(!_hFind) { // first call to method
		if((_hFind = ::FindFirstFile(_pattern, &_wfd)) == INVALID_HANDLE_VALUE) { // init iteration
			_hFind = NULL;
			return NULL; // no files found at all
		}
	} else { // subsequent calls
		if(!::FindNextFile(_hFind, &_wfd)) {
			FindClose(_hFind);
			_hFind = NULL;
			return NULL; // search finished
		}
	}

	const wchar_t *pBackslash;
	if(pBackslash = ::wcsrchr(_pattern, L'\\')) { // search last backslash on user pattern
		int dirnameLen = (int)(pBackslash - _pattern) + 1; // length of directory plus backslash
		::lstrcpyn(buf, _pattern, dirnameLen + 1); // number of chars includes the terminating null
		::lstrcat(buf, _wfd.cFileName); // filepath + filename
	} else {
		::lstrcpy(buf, _wfd.cFileName); // simply copy
	}
	
	return buf; // same passed buffer
}

String* File::Enum::next(String *pBuf)
{
	wchar_t stackbuf[MAX_PATH];
	if(this->next(stackbuf)) {
		*pBuf = stackbuf;
		return pBuf;
	}
	*pBuf = L"";
	return NULL; // search finished
}