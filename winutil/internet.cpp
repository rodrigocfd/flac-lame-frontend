/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#include "internet.h"
#include "str.h"
#pragma comment(lib, "Winhttp.lib")
using namespace winutil;
using std::initializer_list;
using std::unordered_map;
using std::vector;
using std::wstring;

static wstring _formatError(DWORD lastError, const wchar_t *funcName)
{
	const wchar_t *s = nullptr;

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


internet_session::internet_session()
	: _hSession(nullptr)
{
}

internet_session::internet_session(internet_session&& is)
	: _hSession(is._hSession)
{
	is._hSession = nullptr;
}

internet_session& internet_session::operator=(internet_session&& is)
{
	std::swap(_hSession, is._hSession);
	return *this;
}

void internet_session::close()
{
	if (_hSession) {
		WinHttpCloseHandle(_hSession);
		_hSession = nullptr;
	}
}

bool internet_session::init(wstring *pErr, const wchar_t *userAgent)
{
	if (!_hSession) {
		// http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/45ccd91c-6794-4f9b-8f4f-865c76cc146d
		if (!WinHttpCheckPlatform()) {
			if (pErr) *pErr = L"WinHttpCheckPlatform() failed. This platform is not supported by WinHTTP.";
			return false;
		}

		_hSession = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!_hSession) {
			if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpOpen");
			return false;
		}
	}

	if (pErr) pErr->clear();
	return true;
}


internet_download::internet_download(const internet_session& session, wstring url, wstring verb)
	: _session(session), _hConnect(nullptr), _hRequest(nullptr),
	_contentLength(0), _totalDownloaded(0), _url(url), _verb(verb)
{
}

void internet_download::abort()
{
	if (_hRequest) {
		WinHttpCloseHandle(_hRequest);
		_hRequest = nullptr;
	}
	if (_hConnect) {
		WinHttpCloseHandle(_hConnect);
		_hConnect = nullptr;
	}
}

internet_download& internet_download::add_request_header(const wchar_t *requestHeader)
{
	_requestHeaders.emplace_back(requestHeader);
	return *this;
}

internet_download& internet_download::add_request_header(initializer_list<const wchar_t*> requestHeaders)
{
	for (const wchar_t *rh : requestHeaders) {
		add_request_header(rh);
	}
	return *this;
}

internet_download& internet_download::set_referrer(const wchar_t *referrer)
{
	_referrer = referrer;
	return *this;
}

bool internet_download::start(wstring *pErr)
{
	if (_hConnect) {
		if (pErr) *pErr = L"A download is already in progress.";
		return false;
	}

	if (!_init_handles(pErr) ||
		!_contact_server(pErr) ||
		!_parse_headers(pErr) ) return false;

	_buffer.clear();
	if (pErr) pErr->clear();
	return true;
}

bool internet_download::has_data(wstring *pErr)
{
	// Receive the data from server; user must call this until false.
	DWORD incomingBytes = 0;
	if (!_get_incoming_byte_count(incomingBytes, pErr)) {
		return false;
	}
	if (!incomingBytes) { // no more bytes to be downloaded
		_buffer.clear();
		abort();
		if (pErr) pErr->clear();
		return false;
	}

	_buffer.resize(incomingBytes); // overwrite buffer, user must collect it each iteration
	if (!_receive_bytes(incomingBytes, pErr)) {
		return false;
	}
	_totalDownloaded += incomingBytes;

	if (pErr) pErr->clear();
	return true; // more data to come, call again
}

float internet_download::get_percent() const
{
	return _contentLength ?
		(static_cast<float>(_totalDownloaded) / _contentLength) * 100 :
		0;
}

bool internet_download::_init_handles(wstring *pErr)
{
	// Crack the URL.
	internet_url crackedUrl;
	if (!crackedUrl.crack(_url, pErr)) {
		return false;
	}

	// Open the connection handle.
	if (!( _hConnect = WinHttpConnect(_session.hsession(), crackedUrl.host(), crackedUrl.port(), 0) )) {
		if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpConnect");
		return false;
	}

	// Build the request handle.
	wstring fullPath = crackedUrl.path_and_extra();
	_hRequest = WinHttpOpenRequest(_hConnect, _verb.c_str(),
		fullPath.c_str(), nullptr,
		_referrer.empty() ? WINHTTP_NO_REFERER : _referrer.c_str(),
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		crackedUrl.is_https() ? WINHTTP_FLAG_SECURE : 0);
	if (!_hRequest) {
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpOpenRequest");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool internet_download::_contact_server(wstring *pErr)
{
	// Add the request headers to request handle.
	for (wstring& rh : _requestHeaders) {
		if (!WinHttpAddRequestHeaders(_hRequest, rh.c_str(), static_cast<ULONG>(-1L), WINHTTP_ADDREQ_FLAG_ADD)) {
			DWORD dwErr = GetLastError();
			abort();
			if (pErr) *pErr = _formatError(dwErr, L"WinHttpAddRequestHeaders");
			return false;
		}
	}

	// Send the request to server.
	if (!WinHttpSendRequest(_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpSendRequest");
		return false;
	}

	// Receive the response from server.
	if (!WinHttpReceiveResponse(_hRequest, nullptr)) {
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpReceiveResponse");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool internet_download::_parse_headers(wstring *pErr)
{
	// Retrieve the response header.
	DWORD dwSize = 0;
	WinHttpQueryHeaders(_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);

	wstring rawReh; // raw response headers
	rawReh.resize(dwSize / sizeof(wchar_t));

	if (!WinHttpQueryHeaders(_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		&rawReh[0], &dwSize, WINHTTP_NO_HEADER_INDEX))
	{
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpQueryHeaders");
		return false;
	}

	// Parse the raw response headers into an associative array.
	_responseHeaders.clear();
	vector<wstring> lines = str::explode(rawReh, L"\r\n");

	for (wstring& line : lines) {
		if (line.empty()) {
			continue;
		}
		size_t colonIdx = line.find_first_of(L':');
		if (colonIdx == wstring::npos) { // not a key/value pair, probably response line
			_responseHeaders.emplace(L"", line); // empty key
		} else {
			_responseHeaders.emplace(
				str::trim( line.substr(0, colonIdx) ),
				str::trim( line.substr(colonIdx + 1, line.length() - (colonIdx + 1)) )
			);
		}
	}

	// Retrieve content length, if informed by server.
	if (_responseHeaders.find(L"Content-Length") != _responseHeaders.end()) {
		const wstring& strContentLength = _responseHeaders[L"Content-Length"];
		if (str::is_uint(strContentLength)) { // yes, server informed content length
			_contentLength = std::stoi(strContentLength);
		}
	}

	if (pErr) pErr->clear();
	return true;
}

bool internet_download::_get_incoming_byte_count(DWORD& count, wstring *pErr)
{
	DWORD dwSize = 0;
	if (!WinHttpQueryDataAvailable(_hRequest, &dwSize)) { // how many bytes are about to come
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpQueryDataAvailable");
		return false;
	}
	count = dwSize;
	if (pErr) pErr->clear();
	return true;
}

bool internet_download::_receive_bytes(UINT nBytesToRead, wstring *pErr)
{
	DWORD dwRead = 0;
	if (!WinHttpReadData(_hRequest, static_cast<void*>(&_buffer[0]), nBytesToRead, &dwRead)) {
		DWORD dwErr = GetLastError();
		abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpReadData");
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}


internet_url::internet_url()
{
	SecureZeroMemory(&_uc, sizeof(_uc));
	*_scheme = L'\0';
	*_host   = L'\0';
	*_user   = L'\0';
	*_pwd    = L'\0';
	*_path   = L'\0';
	*_extra  = L'\0';
}

bool internet_url::crack(const wchar_t *address, wstring *pErr)
{
	// This helper class simply breaks an URL address into several parts.

	SecureZeroMemory(&_uc, sizeof(_uc));
	_uc.dwStructSize = sizeof(_uc);

	_uc.lpszScheme    = _scheme; _uc.dwSchemeLength    = ARRAYSIZE(_scheme);
	_uc.lpszHostName  = _host;   _uc.dwHostNameLength  = ARRAYSIZE(_host);
	_uc.lpszUserName  = _user;   _uc.dwUserNameLength  = ARRAYSIZE(_user);
	_uc.lpszPassword  = _pwd;    _uc.dwPasswordLength  = ARRAYSIZE(_pwd);
	_uc.lpszUrlPath   = _path;   _uc.dwUrlPathLength   = ARRAYSIZE(_path);
	_uc.lpszExtraInfo = _extra;  _uc.dwExtraInfoLength = ARRAYSIZE(_extra);

	if (!WinHttpCrackUrl(address, 0, 0, &_uc)) {
		if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpCrackUrl");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

wstring internet_url::path_and_extra() const
{
	wstring ret = _path;
	ret.append(_extra);
	return ret;
}