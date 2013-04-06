//
// Internet handling.
// Afternoon of Saturday, December 1, 2012.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"
#include <WinInet.h>
#pragma comment(lib, "WinInet.lib")

class Internet {
public:
	Internet() :_hInternet(0), _szBuf(4096) { }
	~Internet() { end(); }
	Internet& setBufferSize(DWORD numBytes) { _szBuf = numBytes; return *this; }

	void end() {
		if(_hInternet) ::InternetCloseHandle(_hInternet);
		_hInternet = 0;
	}

	bool begin(const wchar_t *agent, String *pErr=0) {
		if(!(_hInternet = ::InternetOpen(agent, INTERNET_OPEN_TYPE_PRECONFIG, 0, 0, 0))) {
			if(pErr) pErr->fmt(L"InternetOpen() failed, error code %d.", ::GetLastError());
			return false;
		}
		if(pErr) *pErr = L"";
		return true;
	}

	bool request(const wchar_t *url, Array<BYTE> *pBuf, String *pErr=0) {
		HINTERNET hAddr = ::InternetOpenUrl(_hInternet, url, 0, 0,
			INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);
		if(!hAddr) {
			if(pErr) pErr->fmt(L"InternetOpenUrl() failed, error code %d.", ::GetLastError());
			return false;
		}
		pBuf->realloc(_szBuf);
		DWORD bytesRead = 0;
		while(::InternetReadFile(hAddr, pBuf->ptr(pBuf->size() - _szBuf), _szBuf, &bytesRead) && bytesRead)
			pBuf->realloc(pBuf->size() + _szBuf - (_szBuf - bytesRead));
		pBuf->realloc(pBuf->size() - (_szBuf - bytesRead));
		::InternetCloseHandle(hAddr);
		return true;
	}
private:
	HINTERNET _hInternet;
	DWORD     _szBuf;
};