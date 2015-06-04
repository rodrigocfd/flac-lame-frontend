/*!
 * @file
 * @brief Automation for internet operations.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <Windows.h>
#include <winhttp.h>

namespace wolf {

namespace net {
	/// An internet session to be used for download operations.
	class Session final {
	private:
		HINTERNET _hSession;
	public:
		Session() : _hSession(nullptr) { }
		~Session() { close(); }

		HINTERNET hSession() const { return _hSession; }
		void      close()          { if (_hSession) { ::WinHttpCloseHandle(_hSession); _hSession = nullptr; } }
		bool      init(std::wstring *pErr=nullptr, const wchar_t *userAgent=L"TOOLOW/1.0");
	};

	/// A single download operation.
	class Download final {
	private:
		const Session&    _session;
		HINTERNET         _hConnect, _hRequest;
		int               _contentLength, _totalDownloaded;
		std::wstring      _url, _verb, _referrer;
		std::vector<BYTE> _buffer;
		std::vector<std::wstring> _requestHeaders;
		std::unordered_map<std::wstring, std::wstring> _responseHeaders;
	public:
		Download(const Session& session, std::wstring url, std::wstring verb=L"GET") :
			_session(session), _hConnect(nullptr), _hRequest(nullptr),
			_contentLength(0), _totalDownloaded(0), _url(url), _verb(verb) { }
		~Download() { abort(); }

		void      abort();
		Download& addRequestHeaders(std::initializer_list<const wchar_t*> requestHeaders);
		Download& setReferrer(const wchar_t *referrer) { _referrer = referrer; return *this; }
		bool      start(std::wstring *pErr=nullptr);
		bool      hasData(std::wstring *pErr=nullptr);
		int       getContentLength() const   { return _contentLength; }
		int       getTotalDownloaded() const { return _totalDownloaded; }
		float     getPercent() const         { return _contentLength ? (static_cast<float>(_totalDownloaded) / _contentLength) * 100 : 0; }
		const std::vector<BYTE>& getBuffer() const { return _buffer; }
		const std::vector<std::wstring>& getRequestHeaders() const { return _requestHeaders; }
		const std::unordered_map<std::wstring, std::wstring>&  getResponseHeaders() const { return _responseHeaders; }
	private:
		bool _initHandles(std::wstring *pErr=nullptr);
		bool _contactServer(std::wstring *pErr=nullptr);
		bool _parseHeaders(std::wstring *pErr=nullptr);
		bool _getIncomingByteCount(DWORD& count, std::wstring *pErr=nullptr);
		bool _receiveBytes(UINT nBytesToRead, std::wstring *pErr=nullptr);
	};

	/// Cracks an URL into its components.
	class Url final {
	public:
		bool           crack(const wchar_t *address, DWORD *dwErr=nullptr);
		bool           crack(const std::wstring& address, DWORD *dwErr=nullptr) { return crack(address.c_str(), dwErr); }
		const wchar_t* scheme() const       { return _scheme; }
		const wchar_t* host() const         { return _host; }
		const wchar_t* user() const         { return _user; }
		const wchar_t* pwd() const          { return _pwd; }
		const wchar_t* path() const         { return _path; }
		const wchar_t* extra() const        { return _extra; }
		std::wstring   pathAndExtra() const { std::wstring ret = _path; ret.append(_extra); return ret; }
		int            port() const         { return _uc.nPort; }
		bool           isHttps() const      { return _uc.nScheme == INTERNET_SCHEME_HTTPS; }
	private:
		URL_COMPONENTS _uc;
		wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
	};
}//namespace net

}//namespace wolf