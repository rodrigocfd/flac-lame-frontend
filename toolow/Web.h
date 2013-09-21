//
// Automation for internet related operations.
// Night of Monday, June 10, 2013.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "Hash.h"
#include "Ptr.h"
#include "Thread.h"
#include <Winhttp.h>

namespace Web
{
	class ErrorBase {
	public:
		virtual ~ErrorBase() = 0;
	protected:
		static void Format(const wchar_t *funcName, DWORD code, String *pBuf);
	};

	class Connection : protected ErrorBase {
	public:
		explicit Connection(const wchar_t *userAgent) : _userAgent(userAgent), _hSession(0) { }
		void      disconnect();
		bool      connect(String *pErr=0);
		HINTERNET hSession() const { return _hSession; }
	private:
		String    _userAgent;
		HINTERNET _hSession;
	};

	class Downloader : protected ErrorBase {
	public:
		explicit Downloader(Ptr<Connection> pCon)           : _pCon(pCon), _pReferrer(new String), _pRequestHeaders(new Array<String>), _hWndNotify(0), _msgNotify(0) { }
		explicit Downloader(const wchar_t *userAgent)       : _pCon(new Connection(userAgent)), _pReferrer(new String), _pRequestHeaders(new Array<String>), _hWndNotify(0), _msgNotify(0) { _pCon->connect(); }
		Downloader& setReferrer(const wchar_t *referrer)    { *_pReferrer = referrer; return *this; }
		Downloader& addRequestHeader(const wchar_t *header) { _pRequestHeaders->append(header); return *this; }
		Downloader& registerNotify(HWND hWnd, UINT msgCode) { _hWndNotify = hWnd; _msgNotify = msgCode; return *this; }
		bool        download(const wchar_t *address, const wchar_t *verb=L"GET");
	private:
		Ptr<Connection>    _pCon;
		Ptr<String>        _pReferrer;
		Ptr<Array<String>> _pRequestHeaders;
		HWND               _hWndNotify;
		UINT               _msgNotify;
	private:
		class _Worker : public Thread, private ErrorBase {
		public:
			_Worker(Ptr<Connection> pCon, Ptr<String> pReferrer, Ptr<Array<String>> pRequestHeaders, HWND hWndNotify, UINT msgNotify, const wchar_t *address, const wchar_t *verb)
				: _pCon(pCon), _pReferrer(pReferrer), _pRequestHeaders(pRequestHeaders), _hWndNotify(hWndNotify), _msgNotify(msgNotify), _address(address), _verb(verb) { }
		private:
			void onRun();
			void _BuildResponseHeader(const String *rh, Hash<String> *pHash);
			Ptr<Connection>    _pCon;
			Ptr<String>        _pReferrer;
			Ptr<Array<String>> _pRequestHeaders;
			HWND               _hWndNotify;
			UINT               _msgNotify;
			String             _address, _verb;
		};
	};

	class Url {
	public:
		explicit Url(const wchar_t *address);
		const wchar_t *scheme() const        { return _scheme; }
		const wchar_t *host() const          { return _host; }
		const wchar_t *user() const          { return _user; }
		const wchar_t *pwd() const           { return _pwd; }
		const wchar_t *path() const          { return _path; }
		const wchar_t *extra() const         { return _extra; }
		void pathAndExtra(String *buf) const { *buf = _path; buf->append(_extra); }
		int  port() const                    { return _uc.nPort; }
		bool isHttps() const                 { return _uc.nScheme == INTERNET_SCHEME_HTTPS; }
	private:
		URL_COMPONENTS _uc;
		wchar_t _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
	};

	class Status {
	public:
		enum Code { STARTED=0, PROGRESS=1, DONE=2, FAILED=3 };
		explicit Status(Code theCode) : code(theCode), pctDone(0) { }
		Code              code;
		String            msg;
		float             pctDone;
		Ptr<Hash<String>> pResponseHeader;
		Ptr<Array<BYTE>>  pBuffer;
	};
};