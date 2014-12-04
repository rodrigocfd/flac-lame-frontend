//
// Automation for internet related operations.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "Hash.h"
#include <winhttp.h>

struct Internet
{
	class Session final {
	private:
		HINTERNET _hSession;
	public:
		Session() : _hSession(nullptr) { }
		~Session() { close(); }

		HINTERNET hSession() const { return _hSession; }
		void close() { if (_hSession) { ::WinHttpCloseHandle(_hSession); _hSession = nullptr; } }
		bool init(String *pErr=nullptr, const wchar_t *userAgent=L"TOOLOW/1.0");
	};

	class Download final {
	private:
		const Session& _session;
		HINTERNET      _hConnect, _hRequest;
		Array<String>  _requestHeaders;
		Hash<String>   _responseHeaders;
		int            _contentLength, _totalDownloaded;
		String         _url, _verb, _referrer;
		Array<BYTE>    _buffer;
	public:
		Download(const Session& session, String url, String verb=L"GET") :
			_session(session), _hConnect(nullptr), _hRequest(nullptr),
			_contentLength(0), _totalDownloaded(0), _url(url), _verb(verb) { }
		~Download() { abort(); }

		void                 abort();
		Download&            addRequestHeaders(initializer_list<const wchar_t*> requestHeaders);
		Download&            setReferrer(const wchar_t *referrer) { _referrer = referrer; return *this; }
		bool                 start(String *pErr=nullptr);
		bool                 hasData(String *pErr=nullptr);		
		int                  getContentLength() const   { return _contentLength; }
		int                  getTotalDownloaded() const { return _totalDownloaded; }
		float                getPercent() const         { return _contentLength ? ((float)_totalDownloaded / _contentLength) * 100 : 0; }
		const Array<String>& getRequestHeaders() const  { return _requestHeaders; }
		const Hash<String>&  getResponseHeaders() const { return _responseHeaders; }
		const Array<BYTE>&   getBuffer() const          { return _buffer; }
	private:
		bool _initHandles(String *pErr=nullptr);
		bool _contactServer(String *pErr=nullptr);
		bool _parseHeaders(String *pErr=nullptr);
		bool _getIncomingByteCount(DWORD& count, String *pErr=nullptr);
		bool _receiveBytes(UINT nBytesToRead, String *pErr=nullptr);
	};

private:
	class _Url final {
	public:
		bool           crack(const wchar_t *address, DWORD *dwErr=nullptr);
		bool           crack(const String& address, DWORD *dwErr=nullptr) { return crack(address.str(), dwErr); }
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
		wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
	};

	static String _FormatErr(const wchar_t *funcName, DWORD code);
};