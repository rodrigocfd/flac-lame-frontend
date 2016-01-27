
#include "Internet.h"
#include "Str.h"
#pragma comment(lib, "Winhttp.lib")
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

	return Str::format(L"%s() failed. Error: %s.",
		funcName,
		s ? s : Str::format(L"(unhandled, %08X)", lastError).c_str() );
}


InternetSession::~InternetSession()
{
	this->close();
}

InternetSession::InternetSession()
	: _hSession(nullptr)
{
}

InternetSession::InternetSession(InternetSession&& is)
	: _hSession(is._hSession)
{
	is._hSession = nullptr;
}

InternetSession& InternetSession::operator=(InternetSession&& is)
{
	std::swap(this->_hSession, is._hSession);
	return *this;
}

HINTERNET InternetSession::hSession() const
{
	return this->_hSession;
}

void InternetSession::close()
{
	if (this->_hSession) {
		WinHttpCloseHandle(this->_hSession);
		this->_hSession = nullptr;
	}
}

bool InternetSession::init(wstring *pErr, const wchar_t *userAgent)
{
	if (!this->_hSession) {
		// http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/45ccd91c-6794-4f9b-8f4f-865c76cc146d
		if (!WinHttpCheckPlatform()) {
			if (pErr) *pErr = L"WinHttpCheckPlatform() failed. This platform is not supported by WinHTTP.";
			return false;
		}

		this->_hSession = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		if (!this->_hSession) {
			if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpOpen");
			return false;
		}
	}

	if (pErr) pErr->clear();
	return true;
}


InternetDownload::~InternetDownload()
{
	this->abort();
}

InternetDownload::InternetDownload(const InternetSession& session, wstring url, wstring verb)
	: _session(session), _hConnect(nullptr), _hRequest(nullptr),
	_contentLength(0), _totalDownloaded(0), _url(url), _verb(verb)
{
}

void InternetDownload::abort()
{
	if (this->_hRequest) {
		WinHttpCloseHandle(this->_hRequest);
		this->_hRequest = nullptr;
	}
	if (this->_hConnect) {
		WinHttpCloseHandle(this->_hConnect);
		this->_hConnect = nullptr;
	}
}

InternetDownload& InternetDownload::addRequestHeader(const wchar_t *requestHeader)
{
	this->_requestHeaders.emplace_back(requestHeader);
	return *this;
}

InternetDownload& InternetDownload::addRequestHeader(initializer_list<const wchar_t*> requestHeaders)
{
	for (const wchar_t *rh : requestHeaders) {
		this->addRequestHeader(rh);
	}
	return *this;
}

InternetDownload& InternetDownload::setReferrer(const wchar_t *referrer)
{
	this->_referrer = referrer;
	return *this;
}

bool InternetDownload::start(wstring *pErr)
{
	if (this->_hConnect) {
		if (pErr) *pErr = L"A download is already in progress.";
		return false;
	}

	if (!this->_initHandles(pErr) ||
		!this->_contactServer(pErr) ||
		!this->_parseHeaders(pErr) ) return false;

	this->_buffer.clear();
	if (pErr) pErr->clear();
	return true;
}

bool InternetDownload::hasData(wstring *pErr)
{
	// Receive the data from server; user must call this until false.
	DWORD incomingBytes = 0;
	if (!this->_getIncomingByteCount(incomingBytes, pErr)) {
		return false;
	}
	if (!incomingBytes) { // no more bytes to be downloaded
		this->_buffer.clear();
		this->abort();
		if (pErr) pErr->clear();
		return false;
	}

	this->_buffer.resize(incomingBytes); // overwrite buffer, user must collect it each iteration
	if (!this->_receiveBytes(incomingBytes, pErr)) {
		return false;
	}
	this->_totalDownloaded += incomingBytes;

	if (pErr) pErr->clear();
	return true; // more data to come, call again
}

int InternetDownload::getContentLength() const
{
	return this->_contentLength;
}

int InternetDownload::getTotalDownloaded() const
{
	return this->_totalDownloaded;
}

float InternetDownload::getPercent() const
{
	return this->_contentLength ?
		(static_cast<float>(this->_totalDownloaded) / this->_contentLength) * 100 :
		0;
}

const vector<BYTE>& InternetDownload::getBuffer() const
{
	return this->_buffer;
}

const vector<wstring>& InternetDownload::getRequestHeaders() const
{
	return this->_requestHeaders;
}

const unordered_map<wstring, wstring>& InternetDownload::getResponseHeaders() const
{
	return this->_responseHeaders;
}

bool InternetDownload::_initHandles(wstring *pErr)
{
	// Crack the URL.
	InternetUrl crackedUrl;
	if (!crackedUrl.crack(this->_url, pErr)) {
		return false;
	}

	// Open the connection handle.
	if (!( this->_hConnect = WinHttpConnect(this->_session.hSession(), crackedUrl.host(), crackedUrl.port(), 0) )) {
		if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpConnect");
		return false;
	}

	// Build the request handle.
	wstring fullPath = crackedUrl.pathAndExtra();
	this->_hRequest = WinHttpOpenRequest(this->_hConnect, this->_verb.c_str(),
		fullPath.c_str(), nullptr,
		this->_referrer.empty() ? WINHTTP_NO_REFERER : this->_referrer.c_str(),
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		crackedUrl.isHttps() ? WINHTTP_FLAG_SECURE : 0);
	if (!this->_hRequest) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpOpenRequest");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool InternetDownload::_contactServer(wstring *pErr)
{
	// Add the request headers to request handle.
	for (wstring& rh : this->_requestHeaders) {
		if (!WinHttpAddRequestHeaders(this->_hRequest, rh.c_str(), static_cast<ULONG>(-1L), WINHTTP_ADDREQ_FLAG_ADD)) {
			DWORD dwErr = GetLastError();
			this->abort();
			if (pErr) *pErr = _formatError(dwErr, L"WinHttpAddRequestHeaders");
			return false;
		}
	}

	// Send the request to server.
	if (!WinHttpSendRequest(this->_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpSendRequest");
		return false;
	}

	// Receive the response from server.
	if (!WinHttpReceiveResponse(this->_hRequest, nullptr)) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpReceiveResponse");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool InternetDownload::_parseHeaders(wstring *pErr)
{
	// Retrieve the response header.
	DWORD dwSize = 0;
	WinHttpQueryHeaders(this->_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);

	wstring rawReh; // raw response headers
	rawReh.resize(dwSize / sizeof(wchar_t));

	if (!WinHttpQueryHeaders(this->_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		&rawReh[0], &dwSize, WINHTTP_NO_HEADER_INDEX))
	{
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpQueryHeaders");
		return false;
	}

	// Parse the raw response headers into an associative array.
	this->_responseHeaders.clear();
	vector<wstring> lines = Str::explode(rawReh, L"\r\n");

	for (wstring& line : lines) {
		if (line.empty()) {
			continue;
		}
		size_t colonIdx = line.find_first_of(L':');
		if (colonIdx == wstring::npos) { // not a key/value pair, probably response line
			this->_responseHeaders.emplace(L"", line); // empty key
		} else {
			this->_responseHeaders.emplace(
				Str::trim( line.substr(0, colonIdx) ),
				Str::trim( line.substr(colonIdx + 1, line.length() - (colonIdx + 1)) )
				);
		}
	}

	// Retrieve content length, if informed by server.
	if (this->_responseHeaders.find(L"Content-Length") != this->_responseHeaders.end()) {
		const wstring& strContentLength = this->_responseHeaders[L"Content-Length"];
		if (Str::isUint(strContentLength)) { // yes, server informed content length
			this->_contentLength = std::stoi(strContentLength);
		}
	}

	if (pErr) pErr->clear();
	return true;
}

bool InternetDownload::_getIncomingByteCount(DWORD& count, wstring *pErr)
{
	DWORD dwSize = 0;
	if (!WinHttpQueryDataAvailable(this->_hRequest, &dwSize)) { // how many bytes are about to come
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpQueryDataAvailable");
		return false;
	}
	count = dwSize;
	if (pErr) pErr->clear();
	return true;
}

bool InternetDownload::_receiveBytes(UINT nBytesToRead, wstring *pErr)
{
	DWORD dwRead = 0;
	if (!WinHttpReadData(this->_hRequest, static_cast<void*>(&_buffer[0]), nBytesToRead, &dwRead)) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = _formatError(dwErr, L"WinHttpReadData");
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}


InternetUrl::InternetUrl()
{
	SecureZeroMemory(&this->_uc, sizeof(this->_uc));
	*this->_scheme = L'\0';
	*this->_host   = L'\0';
	*this->_user   = L'\0';
	*this->_pwd    = L'\0';
	*this->_path   = L'\0';
	*this->_extra  = L'\0';
}

bool InternetUrl::crack(const wchar_t *address, wstring *pErr)
{
	// This helper class simply breaks an URL address into several parts.

	SecureZeroMemory(&this->_uc, sizeof(this->_uc));
	this->_uc.dwStructSize = sizeof(this->_uc);

	this->_uc.lpszScheme    = this->_scheme; this->_uc.dwSchemeLength    = ARRAYSIZE(this->_scheme);
	this->_uc.lpszHostName  = this->_host;   this->_uc.dwHostNameLength  = ARRAYSIZE(this->_host);
	this->_uc.lpszUserName  = this->_user;   this->_uc.dwUserNameLength  = ARRAYSIZE(this->_user);
	this->_uc.lpszPassword  = this->_pwd;    this->_uc.dwPasswordLength  = ARRAYSIZE(this->_pwd);
	this->_uc.lpszUrlPath   = this->_path;   this->_uc.dwUrlPathLength   = ARRAYSIZE(this->_path);
	this->_uc.lpszExtraInfo = this->_extra;  this->_uc.dwExtraInfoLength = ARRAYSIZE(this->_extra);

	if (!WinHttpCrackUrl(address, 0, 0, &this->_uc)) {
		if (pErr) *pErr = _formatError(GetLastError(), L"WinHttpCrackUrl");
		return false;
	}

	if (pErr) pErr->clear();
	return true;
}

bool InternetUrl::crack(const wstring& address, wstring *pErr)
{
	return this->crack(address.c_str(), pErr);
}

const wchar_t* InternetUrl::scheme() const
{
	return this->_scheme;
}

const wchar_t* InternetUrl::host() const
{
	return this->_host;
}

const wchar_t* InternetUrl::user() const
{
	return this->_user;
}

const wchar_t* InternetUrl::pwd() const
{
	return this->_pwd;
}

const wchar_t* InternetUrl::path() const
{
	return this->_path;
}

const wchar_t* InternetUrl::extra() const
{
	return this->_extra;
}

wstring InternetUrl::pathAndExtra() const
{
	wstring ret = this->_path;
	ret.append(this->_extra);
	return ret;
}

int InternetUrl::port() const
{
	return this->_uc.nPort;
}

bool InternetUrl::isHttps() const
{
	return this->_uc.nScheme == INTERNET_SCHEME_HTTPS;
}