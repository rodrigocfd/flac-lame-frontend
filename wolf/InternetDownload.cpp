/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "InternetDownload.h"
#include "Str.h"
using namespace wolf;
using std::initializer_list;
using std::unordered_map;
using std::vector;
using std::wstring;

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

InternetDownload& InternetDownload::addRequestHeaders(initializer_list<const wchar_t*> requestHeaders)
{
	for (const wchar_t *rh : requestHeaders) {
		this->_requestHeaders.emplace_back(rh);
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
		if (pErr) *pErr = InternetSession::formatError(GetLastError(), L"WinHttpConnect");
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
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpOpenRequest");
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
			if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpAddRequestHeaders");
			return false;
		}
	}

	// Send the request to server.
	if (!WinHttpSendRequest(this->_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpSendRequest");
		return false;
	}

	// Receive the response from server.
	if (!WinHttpReceiveResponse(this->_hRequest, nullptr)) {
		DWORD dwErr = GetLastError();
		this->abort();
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpReceiveResponse");
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
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpQueryHeaders");
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
		if (Str::isIntU(strContentLength)) { // yes, server informed content length
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
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpQueryDataAvailable");
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
		if (pErr) *pErr = InternetSession::formatError(dwErr, L"WinHttpReadData");
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}