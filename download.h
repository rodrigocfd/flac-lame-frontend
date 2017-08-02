/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <Windows.h>
#include <winhttp.h>
#include "dictionary.h"
#include "str.h"
#pragma comment(lib, "Winhttp.lib")

namespace wl {

// Wrapper to manage Internet downloads.
class download final {
public:
	class session final {
	private:
		HINTERNET _hSession;
	public:
		~session() { this->close(); }
		session() : _hSession(nullptr) { }
		session(session&& other) : _hSession(other._hSession) { other._hSession = nullptr; }
		HINTERNET hsession() const { return _hSession; }

		session& operator=(session&& is) {
			std::swap(this->_hSession, is._hSession);
			return *this;
		}

		void close() {
			if (this->_hSession) {
				WinHttpCloseHandle(this->_hSession);
				this->_hSession = nullptr;
			}
		}

		bool open(std::wstring* pErr = nullptr, const wchar_t* userAgent = L"Win32/1.0") {
			if (!this->_hSession) {
				// http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/45ccd91c-6794-4f9b-8f4f-865c76cc146d
				if (!WinHttpCheckPlatform()) {
					if (pErr) *pErr = L"WinHttpCheckPlatform() failed. This platform is not supported by WinHTTP.";
					return false;
				}

				this->_hSession = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
				if (!this->_hSession) {
					if (pErr) *pErr = _format_error(GetLastError(), L"WinHttpOpen");
					return false;
				}
			}

			if (pErr) pErr->clear();
			return true;
		}
	};

	class url_crack final {
	private:
		URL_COMPONENTS _uc;
		wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
	public:
		url_crack() {
			SecureZeroMemory(&this->_uc, sizeof(this->_uc));
			*this->_scheme = L'\0';
			*this->_host   = L'\0';
			*this->_user   = L'\0';
			*this->_pwd    = L'\0';
			*this->_path   = L'\0';
			*this->_extra  = L'\0';
		}

		bool crack(const std::wstring& address, std::wstring* pErr = nullptr) {
			// This helper class simply breaks an URL address into several parts.
			SecureZeroMemory(&this->_uc, sizeof(this->_uc));
			this->_uc.dwStructSize = sizeof(this->_uc);

			this->_uc.lpszScheme    = this->_scheme; this->_uc.dwSchemeLength    = ARRAYSIZE(this->_scheme);
			this->_uc.lpszHostName  = this->_host;   this->_uc.dwHostNameLength  = ARRAYSIZE(this->_host);
			this->_uc.lpszUserName  = this->_user;   this->_uc.dwUserNameLength  = ARRAYSIZE(this->_user);
			this->_uc.lpszPassword  = this->_pwd;    this->_uc.dwPasswordLength  = ARRAYSIZE(this->_pwd);
			this->_uc.lpszUrlPath   = this->_path;   this->_uc.dwUrlPathLength   = ARRAYSIZE(this->_path);
			this->_uc.lpszExtraInfo = this->_extra;  this->_uc.dwExtraInfoLength = ARRAYSIZE(this->_extra);

			if (!WinHttpCrackUrl(address.c_str(), 0, 0, &this->_uc)) {
				if (pErr) *pErr = _format_error(GetLastError(), L"WinHttpCrackUrl");
				return false;
			}
			if (pErr) pErr->clear();
			return true;
		}

		const wchar_t* scheme() const   { return this->_scheme; }
		const wchar_t* host() const     { return this->_host; }
		const wchar_t* user() const     { return this->_user; }
		const wchar_t* pwd() const      { return this->_pwd; }
		const wchar_t* path() const     { return this->_path; }
		const wchar_t* extra() const    { return this->_extra; }
		int            port() const     { return this->_uc.nPort; }
		bool           is_https() const { return this->_uc.nScheme == INTERNET_SCHEME_HTTPS; }

		std::wstring path_and_extra() const {
			std::wstring ret = this->_path;
			ret.append(this->_extra);
			return ret;
		}
	};

private:
	const session&     _session;
	HINTERNET          _hConnect, _hRequest;
	size_t             _contentLength, _totalGot;
	std::wstring       _url, _verb, _referrer;
	dictionary_str_str _requestHeaders;
	dictionary_str_str _responseHeaders;

public:
	std::vector<BYTE> data;

	~download() { this->abort(); }

	download(const session& sess, std::wstring url, std::wstring verb = L"GET")
		: _session(sess), _hConnect(nullptr), _hRequest(nullptr),
			_contentLength(0), _totalGot(0), _url(url), _verb(verb) { }

	void abort() {
		if (this->_hRequest) {
			WinHttpCloseHandle(this->_hRequest);
			this->_hRequest = nullptr;
		}
		if (this->_hConnect) {
			WinHttpCloseHandle(this->_hConnect);
			this->_hConnect = nullptr;
		}
	}

	download& add_request_header(const wchar_t* name, const wchar_t* value) {
		this->_requestHeaders.add(name, value);
		return *this;
	}

	download& set_referrer(const wchar_t* referrer) {
		this->_referrer = referrer;
		return *this;
	}

	download& set_referrer(const std::wstring& referrer) {
		return this->set_referrer(referrer.c_str());
	}

	bool start(std::wstring* pErr = nullptr) {
		if (this->_hConnect) {
			if (pErr) *pErr = L"A download is already in progress.";
			return false;
		}

		this->_contentLength = this->_totalGot = 0;

		if (!this->_init_handles(pErr) ||
			!this->_contact_server(pErr) ||
			!this->_parse_headers(pErr) ) return false;

		this->data.clear(); // prepare buffer to receive data
		if (this->_contentLength) { // server informed content length?
			this->data.reserve(this->_contentLength);
		}
		if (pErr) pErr->clear();
		return true;
	}

	const dictionary_str_str& get_request_headers() const  { return this->_requestHeaders; }
	const dictionary_str_str& get_response_headers() const { return this->_responseHeaders; }
	size_t                    get_content_length() const   { return this->_contentLength; }
	size_t                    get_total_downloaded() const { return this->_totalGot; }

	float get_percent() const {
		return this->_contentLength ?
			(static_cast<float>(this->_totalGot) / this->_contentLength) * 100 :
			0;
	}

	bool has_data(std::wstring* pErr = nullptr) {
		// Receive the data from server; user must call this until false.
		DWORD incomingBytes = 0;
		if (!this->_get_incoming_byte_count(incomingBytes, pErr)) {
			return false;
		}
		if (!incomingBytes) { // no more bytes to be downloaded
			this->abort();
			if (pErr) pErr->clear();
			return false;
		}
		if (!this->_receive_bytes(incomingBytes, pErr)) {
			return false;
		}
		if (pErr) pErr->clear();
		return true; // more data to come, call again
	}

private:
	bool _init_handles(std::wstring* pErr = nullptr) {
		// Crack the URL.
		url_crack crackedUrl;
		if (!crackedUrl.crack(_url, pErr)) {
			return false;
		}

		// Open the connection handle.
		if (!( this->_hConnect = WinHttpConnect(this->_session.hsession(), crackedUrl.host(), crackedUrl.port(), 0) )) {
			if (pErr) *pErr = _format_error(GetLastError(), L"WinHttpConnect");
			return false;
		}

		// Build the request handle.
		std::wstring fullPath = crackedUrl.path_and_extra();
		this->_hRequest = WinHttpOpenRequest(this->_hConnect, this->_verb.c_str(),
			fullPath.c_str(), nullptr,
			this->_referrer.empty() ? WINHTTP_NO_REFERER : this->_referrer.c_str(),
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			crackedUrl.is_https() ? WINHTTP_FLAG_SECURE : 0);
		if (!this->_hRequest) {
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpOpenRequest");
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool _contact_server(std::wstring* pErr = nullptr) {
		// Add the request headers to request handle.
		std::wstring rhTmp;
		rhTmp.reserve(20);
		for (const dictionary_str_str::entry& rh : this->_requestHeaders.entries()) {
			rhTmp = rh.key;
			rhTmp += L": ";
			rhTmp += rh.value;

			if (!WinHttpAddRequestHeaders(this->_hRequest, rhTmp.c_str(), static_cast<ULONG>(-1L), WINHTTP_ADDREQ_FLAG_ADD)) {
				DWORD dwErr = GetLastError();
				this->abort();
				if (pErr) *pErr = _format_error(dwErr, L"WinHttpAddRequestHeaders");
				return false;
			}
		}

		// Send the request to server.
		if (!WinHttpSendRequest(this->_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpSendRequest");
			return false;
		}

		// Receive the response from server.
		if (!WinHttpReceiveResponse(this->_hRequest, nullptr)) {
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpReceiveResponse");
			return false;
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool _parse_headers(std::wstring* pErr = nullptr) {
		// Retrieve the response header.
		DWORD dwSize = 0;
		WinHttpQueryHeaders(this->_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);

		std::wstring rawReh; // raw response headers
		rawReh.resize(dwSize / sizeof(wchar_t));

		if (!WinHttpQueryHeaders(this->_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF,
			WINHTTP_HEADER_NAME_BY_INDEX, &rawReh[0], &dwSize, WINHTTP_NO_HEADER_INDEX))
		{
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpQueryHeaders");
			return false;
		}

		// Parse the raw response headers into an associative array.
		this->_responseHeaders.clear();
		std::vector<std::wstring> lines = str::explode(rawReh, L"\r\n");

		for (std::wstring& line : lines) {
			if (line.empty()) {
				continue;
			}
			size_t colonIdx = line.find_first_of(L':');
			if (colonIdx == std::wstring::npos) { // not a key/value pair, probably response line
				this->_responseHeaders.add(L"", line); // empty key
			} else {
				this->_responseHeaders.add(
					str::trim( line.substr(0, colonIdx) ),
					str::trim( line.substr(colonIdx + 1, line.length() - (colonIdx + 1)) )
				);
			}
		}

		// Retrieve content length, if informed by server.
		std::wstring* contLen = this->_responseHeaders.val(L"Content-Length");
		if (contLen && str::is_uint(*contLen)) { // yes, server informed content length
			this->_contentLength = std::stoul(*contLen);
		}

		if (pErr) pErr->clear();
		return true;
	}

	bool _get_incoming_byte_count(DWORD& count, std::wstring* pErr = nullptr) {
		DWORD dwSize = 0;
		if (!WinHttpQueryDataAvailable(this->_hRequest, &dwSize)) { // how many bytes are about to come
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpQueryDataAvailable");
			return false;
		}
		count = dwSize;
		if (pErr) pErr->clear();
		return true;
	}

	bool _receive_bytes(UINT nBytesToRead, std::wstring* pErr = nullptr) {
		DWORD dwRead = 0;
		this->data.resize(this->data.size() + nBytesToRead); // make room

		if (!WinHttpReadData(this->_hRequest,
			static_cast<void*>(&this->data[this->data.size() - nBytesToRead]), // append to user buffer
			nBytesToRead, &dwRead) )
		{
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _format_error(dwErr, L"WinHttpReadData");
			return false;
		}

		this->_totalGot += nBytesToRead;
		if (pErr) pErr->clear();
		return true;
	}

	static std::wstring _format_error(DWORD lastError, const wchar_t* funcName) {
		const wchar_t* s = nullptr;
		switch (lastError) {
		case ERROR_NOT_ENOUGH_MEMORY:               s = L"not enough memory"; break;
		case ERROR_WINHTTP_CANNOT_CONNECT:          s = L"cannot connect"; break;
		case ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW: s = L"chunked encoding header size overflow"; break;
		case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: s = L"client auth cert needed"; break;
		case ERROR_WINHTTP_CONNECTION_ERROR:        s = L"connection error"; break;
		case ERROR_WINHTTP_HEADER_COUNT_EXCEEDED:   s = L"header count exceeded"; break;
		case ERROR_WINHTTP_HEADER_NOT_FOUND:        s = L"header not found"; break;
		case ERROR_WINHTTP_HEADER_SIZE_OVERFLOW:    s = L"header size overflow"; break;
		case ERROR_WINHTTP_INCORRECT_HANDLE_STATE:  s = L"incorrect handle state"; break;
		case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:   s = L"incorrect handle type"; break;
		case ERROR_WINHTTP_INTERNAL_ERROR:          s = L"internal error"; break;
		case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: s = L"invalid server response"; break;
		case ERROR_WINHTTP_INVALID_URL:             s = L"invalid URL"; break;
		case ERROR_WINHTTP_LOGIN_FAILURE:           s = L"login failure"; break;
		case ERROR_WINHTTP_NAME_NOT_RESOLVED:       s = L"name not resolved"; break;
		case ERROR_WINHTTP_OPERATION_CANCELLED:     s = L"operation cancelled"; break;
		case ERROR_WINHTTP_REDIRECT_FAILED:         s = L"redirect failed"; break;
		case ERROR_WINHTTP_RESEND_REQUEST:          s = L"resend request"; break;
		case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: s = L"response drain overflow"; break;
		case ERROR_WINHTTP_SECURE_FAILURE:          s = L"secure failure"; break;
		case ERROR_WINHTTP_SHUTDOWN:                s = L"shutdown"; break;
		case ERROR_WINHTTP_TIMEOUT:                 s = L"timeout"; break;
		case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:     s = L"unrecognized scheme or bad URL"; break;
		default:                                    s = nullptr;
		}
		return str::format(L"%s() failed. Error: %s.",
			funcName,
			s ? s : str::format(L"(unhandled, %08X)", lastError).c_str() );
	}
};

}//namespace wl