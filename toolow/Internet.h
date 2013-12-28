//
// Automation for internet related operations.
// Night of Monday, June 10, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Hash.h"
#include "Thread.h"
#include <Winhttp.h>

class Internet {
public:
	explicit Internet(const wchar_t *userAgent=L"TOOLOW/1.0") : _hSession(0), _userAgent(userAgent), _hWndNotify(0), _msgNotify(0) { }
	Internet& setUserAgent(const wchar_t *userAgent)        { _userAgent = userAgent; return *this; }
	Internet& setReferrer(const wchar_t *referrer)          { _referrer = referrer; return *this; }
	Internet& addRequestHeader(const wchar_t *header)       { _requestHeaders.append(header); return *this; }
	Internet& registerNotify(HWND hWnd, UINT msgCode)       { _hWndNotify = hWnd; _msgNotify = msgCode; return *this; }
	bool      download(const wchar_t *address, const wchar_t *verb=L"GET", String *pErr=0);

private:
	HINTERNET     _hSession;
	String        _userAgent;
	String        _referrer;
	Array<String> _requestHeaders;
	HWND          _hWndNotify;
	UINT          _msgNotify;

	static void _Format(const wchar_t *funcName, DWORD code, String *pBuf);

private:
	class _Worker : public Thread {
	public:
		_Worker(HINTERNET hSession, const wchar_t *referrer, const Array<String> *pRequestHeaders,
			HWND hWndNotify, UINT msgNotify, const wchar_t *address, const wchar_t *verb)
		: _hSession(hSession), _hConnect(0), _hRequest(0), _referrer(referrer), _requestHeaders(*pRequestHeaders),
			_hWndNotify(hWndNotify), _msgNotify(msgNotify), _address(address), _verb(verb) { }
	private:
		HINTERNET     _hSession, _hConnect, _hRequest;
		String        _referrer;
		Array<String> _requestHeaders;
		HWND          _hWndNotify;
		UINT          _msgNotify;
		String        _address, _verb;

		void onRun();
		void _cleanup();
		void _notifyError(DWORD errCode, const wchar_t *funcName);
		Hash<String> _buildResponseHeader(const String *rh);
	};

public:
	class Status {
	public:
		enum class Flag { STARTED=0, PROGRESS=1, DONE=2, FAILED=3 };
		explicit Status(Flag theFlag) : flag(theFlag), pctDone(0) { }
		Status& operator=(Status&& other) { flag = other.flag; msg = (String&&)other.msg; pctDone = other.pctDone; responseHeader = (Hash<String>&&)other.responseHeader; buffer = (Array<BYTE>&&)other.buffer; }

		Flag         flag;
		String       msg;
		float        pctDone;
		Hash<String> responseHeader;
		Array<BYTE>  buffer;
	};

	class Url {
	public:
		explicit Url(const wchar_t *address);
		const wchar_t* scheme() const       { return _scheme; }
		const wchar_t* host() const         { return _host; }
		const wchar_t* user() const         { return _user; }
		const wchar_t* pwd() const          { return _pwd; }
		const wchar_t* path() const         { return _path; }
		const wchar_t* extra() const        { return _extra; }
		String         pathAndExtra() const { String ret(_path); ret.append(_extra); return ret; }
		int            port() const         { return _uc.nPort; }
		bool           isHttps() const      { return _uc.nScheme == INTERNET_SCHEME_HTTPS; }
	private:
		URL_COMPONENTS _uc;
		wchar_t _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
	};
};