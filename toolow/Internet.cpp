
#include "Internet.h"
#include "util.h"
#pragma comment(lib, "Winhttp.lib")

bool Internet::download(const wchar_t *address, const wchar_t *verb, String *pErr)
{
	if(!WinHttpCheckPlatform()) {
		if(pErr) *pErr = L"This platform is not supported by WinHTTP.";
		return false;
	}

	if(!_hWndNotify || !_msgNotify) {
		if(pErr) (*pErr) = L"No window to receive the notifications.";
		return false; // without a window to receive the stuff, download will be useless
	}

	_hSession = WinHttpOpen(_userAgent.str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(!_hSession) {
		if(pErr) this->_Format(L"WinHttpOpen", GetLastError(), pErr);
		return false;
	}

	_Worker *worker = new _Worker(_hSession, _referrer.str(), &_requestHeaders, _hWndNotify, _msgNotify, address, verb);
	worker->runAsync();
	if(pErr) *pErr = L"";
	return true; // _Worker will be responsible to call WinHttpCloseHandle on _hSession
}

void Internet::_Format(const wchar_t *funcName, DWORD code, String *pBuf)
{
	const wchar_t *s = NULL;
	switch(code) {
	case ERROR_NOT_ENOUGH_MEMORY:               s = L"ERROR_NOT_ENOUGH_MEMORY"; break;
	case ERROR_WINHTTP_CANNOT_CONNECT:          s = L"ERROR_WINHTTP_CANNOT_CONNECT"; break;
	case ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW: s = L"ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW"; break;
	case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: s = L"ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED"; break;
	case ERROR_WINHTTP_CONNECTION_ERROR:        s = L"ERROR_WINHTTP_CONNECTION_ERROR"; break;
	case ERROR_WINHTTP_HEADER_COUNT_EXCEEDED:   s = L"ERROR_WINHTTP_HEADER_COUNT_EXCEEDED"; break;
	case ERROR_WINHTTP_HEADER_NOT_FOUND:        s = L"ERROR_WINHTTP_HEADER_NOT_FOUND"; break;
	case ERROR_WINHTTP_HEADER_SIZE_OVERFLOW:    s = L"ERROR_WINHTTP_HEADER_SIZE_OVERFLOW"; break;
	case ERROR_WINHTTP_INCORRECT_HANDLE_STATE:  s = L"ERROR_WINHTTP_INCORRECT_HANDLE_STATE"; break;
	case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE:   s = L"ERROR_WINHTTP_INCORRECT_HANDLE_TYPE"; break;
	case ERROR_WINHTTP_INTERNAL_ERROR:          s = L"ERROR_WINHTTP_INTERNAL_ERROR"; break;
	case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: s = L"ERROR_WINHTTP_INVALID_SERVER_RESPONSE"; break;
	case ERROR_WINHTTP_INVALID_URL:             s = L"ERROR_WINHTTP_INVALID_URL"; break;
	case ERROR_WINHTTP_LOGIN_FAILURE:           s = L"ERROR_WINHTTP_LOGIN_FAILURE"; break;
	case ERROR_WINHTTP_NAME_NOT_RESOLVED:       s = L"ERROR_WINHTTP_NAME_NOT_RESOLVED"; break;
	case ERROR_WINHTTP_OPERATION_CANCELLED:     s = L"ERROR_WINHTTP_OPERATION_CANCELLED"; break;
	case ERROR_WINHTTP_REDIRECT_FAILED:         s = L"ERROR_WINHTTP_REDIRECT_FAILED"; break;
	case ERROR_WINHTTP_RESEND_REQUEST:          s = L"ERROR_WINHTTP_RESEND_REQUEST"; break;
	case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: s = L"ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW"; break;
	case ERROR_WINHTTP_SECURE_FAILURE:          s = L"ERROR_WINHTTP_SECURE_FAILURE"; break;
	case ERROR_WINHTTP_SHUTDOWN:                s = L"ERROR_WINHTTP_SHUTDOWN"; break;
	case ERROR_WINHTTP_TIMEOUT:                 s = L"ERROR_WINHTTP_TIMEOUT"; break;
	case ERROR_WINHTTP_UNRECOGNIZED_SCHEME:     s = L"ERROR_WINHTTP_UNRECOGNIZED_SCHEME"; break;
	default:                                    s = NULL;
	}
	
	pBuf->format(L"%s() failed. Error: %s.", funcName, s ? s : FMT(L"(unhandled, %d)", code));
}

void Internet::_Worker::onRun()
{	
	// http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/45ccd91c-6794-4f9b-8f4f-865c76cc146d

	Url url(_address.str()); // crack the URL
	
	// Open the connection handle.
	_hConnect = WinHttpConnect(_hSession, url.host(), url.port(), 0);
	if(!_hConnect) {
		DWORD dwErr = GetLastError();
		this->_cleanup();
		this->_notifyError(dwErr, L"WinHttpConnect");
		return;
	}

	// Build the request handle.
	String fullPath = url.pathAndExtra();

	_hRequest = WinHttpOpenRequest(_hConnect, _verb.str(), fullPath.str(), NULL,
		_referrer.isEmpty() ? WINHTTP_NO_REFERER : _referrer.str(),
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		url.isHttps() ? WINHTTP_FLAG_SECURE : 0);
	if(!_hRequest) {
		DWORD dwErr = GetLastError();
		this->_cleanup();
		this->_notifyError(dwErr, L"WinHttpOpenRequest");
		return;
	}

	// Add request headers, if any.
	for(int i = 0; i < _requestHeaders.size(); ++i) {
		if(!WinHttpAddRequestHeaders(_hRequest, _requestHeaders[i].str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD)) {
			DWORD dwErr = GetLastError();
			this->_cleanup();
			this->_notifyError(dwErr, L"WinHttpAddRequestHeaders");
			return;
		}
	}

	// Send the request to server.
	if(!WinHttpSendRequest(_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		DWORD dwErr = GetLastError();
		this->_cleanup();
		this->_notifyError(dwErr, L"WinHttpSendRequest");
		return;
	}

	// Receive the response from server.
	if(!WinHttpReceiveResponse(_hRequest, 0)) {
		DWORD dwErr = GetLastError();
		this->_cleanup();
		this->_notifyError(dwErr, L"WinHttpReceiveResponse");
		return;
	}

	// Status object to be sent on further notifications.
	Status status(Status::Flag::STARTED);

	// Retrieve the response header.
	DWORD dwSize = 0;
	WinHttpQueryHeaders(_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);

	String rawRH;
	rawRH.reserve(dwSize / sizeof(wchar_t) - 1);
	if(!WinHttpQueryHeaders(_hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, rawRH.ptrAt(0), &dwSize, WINHTTP_NO_HEADER_INDEX)) {
		DWORD dwErr = GetLastError();
		this->_cleanup();
		this->_notifyError(dwErr, L"WinHttpQueryHeaders");
		return;
	}
	status.responseHeader = this->_buildResponseHeader(&rawRH);
	SendMessage(_hWndNotify, _msgNotify, (WPARAM)Status::Flag::STARTED, (LPARAM)&status);

	// Check if server informed content length.
	int contentLength = -1;
	if(status.responseHeader.exists(L"Content-Length")) {
		String *strContentLength = &status.responseHeader[L"Content-Length"];
		if(strContentLength->isInt())
			contentLength = strContentLength->toInt();
	}

	// Receive the data from server.
	DWORD dwDownloaded = 0;
	do {
		dwSize = 0;
		if(!WinHttpQueryDataAvailable(_hRequest, &dwSize)) { // how many bytes are about to come
			DWORD dwErr = GetLastError();
			this->_cleanup();
			this->_notifyError(dwErr, L"WinHttpQueryDataAvailable");
			return;
		}
		int prevSz = status.buffer.size();
		status.buffer.realloc(prevSz + dwSize); // grow buffer
		if(!WinHttpReadData(_hRequest, (void*)&status.buffer[prevSz], dwSize, &dwDownloaded)) { // receive the bytes
			DWORD dwErr = GetLastError();
			this->_cleanup();
			this->_notifyError(dwErr, L"WinHttpReadData");
			return;
		}
		status.flag = Status::Flag::PROGRESS;
		if(contentLength > -1)
			status.pctDone = (float)status.buffer.size() / contentLength;
		SendMessage(_hWndNotify, _msgNotify, (WPARAM)Status::Flag::PROGRESS, (LPARAM)&status);
	} while(dwSize > 0);

	this->_cleanup();

	status.flag = Status::Flag::DONE;
	status.pctDone = 1;
	SendMessage(_hWndNotify, _msgNotify, (WPARAM)Status::Flag::DONE, (LPARAM)&status);
}

void Internet::_Worker::_cleanup()
{
	if(_hRequest) {	
		WinHttpCloseHandle(_hRequest);
		_hRequest = NULL;
	}
	if(_hConnect) {	
		WinHttpCloseHandle(_hConnect);
		_hConnect = NULL;
	}
	if(_hSession) {	
		WinHttpCloseHandle(_hSession);
		_hSession = NULL;
	}
}

void Internet::_Worker::_notifyError(DWORD errCode, const wchar_t *funcName)
{
	Status status(Status::Flag::FAILED);
	Internet::_Format(funcName, errCode, &status.msg);
	SendMessage(_hWndNotify, _msgNotify,
		(WPARAM)Status::Flag::FAILED, // this thread will be blocked until SendMessage returns
		(LPARAM)&status); // send pointer to Status object
}

Hash<String> Internet::_Worker::_buildResponseHeader(const String *rh)
{
	Hash<String>  hash;
	Array<String> lines = rh->explode(L"\r\n");
	String        key, val; // declared here to save reallocs

	for(int i = 0; i < lines.size(); ++i) {
		if(!lines[i].len()) continue;
		int colonIdx = lines[i].find(L':');
		if(colonIdx == -1) { // not a key/value pair, probably response line
			hash[L""] = lines[i]; // empty key
		} else {
			key.copyFrom(lines[i].ptrAt(0), colonIdx);
			val.copyFrom(lines[i].ptrAt(colonIdx + 1), lines[i].len() - (colonIdx + 1));
			hash[key.trim()] = val.trim();
		}
	}

	return hash;
}

Internet::Url::Url(const wchar_t *address)
{
	SecureZeroMemory(&_uc, sizeof(_uc));
	_uc.dwStructSize = sizeof(_uc);

	_uc.lpszScheme    = _scheme; _uc.dwSchemeLength    = ARRAYSIZE(_scheme);
	_uc.lpszHostName  = _host;   _uc.dwHostNameLength  = ARRAYSIZE(_host);
	_uc.lpszUserName  = _user;   _uc.dwUserNameLength  = ARRAYSIZE(_user);
	_uc.lpszPassword  = _pwd;    _uc.dwPasswordLength  = ARRAYSIZE(_pwd);
	_uc.lpszUrlPath   = _path;   _uc.dwUrlPathLength   = ARRAYSIZE(_path);
	_uc.lpszExtraInfo = _extra;  _uc.dwExtraInfoLength = ARRAYSIZE(_extra);

	WinHttpCrackUrl(address, 0, 0, &_uc);
}