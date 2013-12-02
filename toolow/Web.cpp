
#include "Web.h"
#include "util.h"
#pragma comment(lib, "Winhttp.lib")

Web::ErrorBase::~ErrorBase()
{
}

void Web::ErrorBase::Format(const wchar_t *funcName, DWORD code, String *pBuf)
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
	
	pBuf->fmt(L"%s() failed. Error: %s.", funcName, s ? s : fmt(L"(unhandled, %d)", code)->str());
}

void Web::Connection::disconnect()
{
	if(_hSession) {
		WinHttpCloseHandle(_hSession);
		_hSession = NULL;
	}
}

bool Web::Connection::connect(String *pErr)
{
	if(!WinHttpCheckPlatform()) {
		*pErr = L"This platform is not supported by WinHTTP.";
		return false;
	}

	this->disconnect();
	_hSession = WinHttpOpen(_userAgent.str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if(!_hSession) {
		if(pErr) ErrorBase::Format(L"WinHttpOpen", GetLastError(), pErr);
		return false;
	}
	if(pErr) *pErr = L"";
	return true;
}

bool Web::Downloader::download(const wchar_t *address, const wchar_t *verb, String *pErr)
{
	if(!_hWndNotify || !_msgNotify)
		return false; // without a window to receive the stuff, download will be useless

	if(!_pCon->hSession()) // if not connected yet, attempts to
		if(!_pCon->connect(pErr))
			return false;

	_Worker *dlw = new _Worker(_pCon, _pReferrer, _pRequestHeaders, _hWndNotify, _msgNotify, address, verb);
	dlw->runAsync();
	if(pErr) *pErr = L"";
	return true;
}

Web::Url::Url(const wchar_t *address)
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

void Web::Downloader::_Worker::onRun()
{
	if(!_hWndNotify || !_msgNotify)
		return; // without a window to receive the stuff, download will be useless
	
	if(!_pCon->hSession()) {
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		pStatus->msg = L"WinHttpOpen() call failed for some reason, no session handle.";
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}
	
	// http://social.msdn.microsoft.com/forums/en-US/vclanguage/thread/45ccd91c-6794-4f9b-8f4f-865c76cc146d

	Url url(_address.str()); // crack the URL
	
	// Open the connection handle.
	HINTERNET hConnect = WinHttpConnect(_pCon->hSession(), url.host(), url.port(), 0);
	if(!hConnect) {
		DWORD dwErr = GetLastError();
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		ErrorBase::Format(L"WinHttpConnect", dwErr, &pStatus->msg);
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}

	// Build the request handle.
	String fullPath;
	url.pathAndExtra(&fullPath);

	HINTERNET hRequest = WinHttpOpenRequest(hConnect, _verb.str(), fullPath.str(), NULL,
		_pReferrer->len() ? _pReferrer->str() : WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		url.isHttps() ? WINHTTP_FLAG_SECURE : 0);
	if(!hRequest) {
		DWORD dwErr = GetLastError();
		WinHttpCloseHandle(hConnect);
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		ErrorBase::Format(L"WinHttpOpenRequest", dwErr, &pStatus->msg);
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}

	// Add request headers, if any.
	for(int i = 0; i < _pRequestHeaders->size(); ++i) {
		if(!WinHttpAddRequestHeaders(hRequest, (*_pRequestHeaders)[i].str(), (ULONG)-1L, WINHTTP_ADDREQ_FLAG_ADD)) {
			DWORD dwErr = GetLastError();
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
			ErrorBase::Format(L"WinHttpAddRequestHeaders", dwErr, &pStatus->msg);
			PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
			return;
		}
	}

	// Send the request to server.
	if(!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
		DWORD dwErr = GetLastError();
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		ErrorBase::Format(L"WinHttpSendRequest", dwErr, &pStatus->msg);
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}

	// Receive the response from server.
	if(!WinHttpReceiveResponse(hRequest, 0)) {
		DWORD dwErr = GetLastError();
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		ErrorBase::Format(L"WinHttpReceiveResponse", dwErr, &pStatus->msg);
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}

	// Retrieve the response header.
	DWORD dwSize = 0;
	WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX,
		WINHTTP_NO_OUTPUT_BUFFER, &dwSize, WINHTTP_NO_HEADER_INDEX);

	String rawRH;
	rawRH.reserve(dwSize / sizeof(wchar_t) - 1);
	if(!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, rawRH.ptrAt(0), &dwSize, WINHTTP_NO_HEADER_INDEX)) {
		DWORD dwErr = GetLastError();
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
		ErrorBase::Format(L"WinHttpQueryHeaders", dwErr, &pStatus->msg);
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
		return;
	}
	Ptr<Hash<String>> pResponseHeader(new Hash<String>); // will be sent on all further status notifications
	_Worker::_BuildResponseHeader(&rawRH, pResponseHeader);

	Web::Status *pStartStatus = new Web::Status(Web::Status::STARTED); // should be consumed by receiver
	pStartStatus->pResponseHeader = pResponseHeader;
	PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStartStatus);

	// Check if server informed content length.
	int contentLength = -1;
	if(pResponseHeader->exists(L"Content-Length")) {
		String *strContentLength = &(*pResponseHeader)[L"Content-Length"];
		if(strContentLength->isInt())
			contentLength = strContentLength->toInt();
	}

	// Receive the data from server.
	Ptr<Array<BYTE>> pBuffer(new Array<BYTE>); // will be sent on all further status notifications
	DWORD dwDownloaded = 0;
	do {
		dwSize = 0;
		if(!WinHttpQueryDataAvailable(hRequest, &dwSize)) { // how many bytes are about to come
			DWORD dwErr = GetLastError();
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
			ErrorBase::Format(L"WinHttpQueryDataAvailable", dwErr, &pStatus->msg);
			pStatus->pResponseHeader = pResponseHeader;
			pStatus->pBuffer = pBuffer;
			PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
			return;
		}
		int prevSz = pBuffer->size();
		pBuffer->realloc(prevSz + dwSize); // grow buffer
		if(!WinHttpReadData(hRequest, (void*)&(*pBuffer)[prevSz], dwSize, &dwDownloaded)) { // receive the bytes
			DWORD dwErr = GetLastError();
			WinHttpCloseHandle(hRequest);
			WinHttpCloseHandle(hConnect);
			Web::Status *pStatus = new Status(Web::Status::FAILED); // should be consumed by receiver
			ErrorBase::Format(L"WinHttpReadData", dwErr, &pStatus->msg);
			pStatus->pResponseHeader = pResponseHeader;
			pStatus->pBuffer = pBuffer;
			PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatus);
			return;
		}
		Web::Status *pStatusProgress = new Web::Status(Web::Status::PROGRESS); // should be consumed by receiver
		pStatusProgress->pResponseHeader = pResponseHeader;
		pStatusProgress->pBuffer = pBuffer;
		if(contentLength > -1)
			pStatusProgress->pctDone = (float)pBuffer->size() / contentLength;
		PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatusProgress);
	} while(dwSize > 0);

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);

	Web::Status *pStatusDone = new Web::Status(Web::Status::DONE); // should be consumed by receiver
	pStatusDone->pResponseHeader = pResponseHeader;
	pStatusDone->pBuffer = pBuffer;
	pStatusDone->pctDone = 1;
	PostMessage(_hWndNotify, _msgNotify, 0, (LPARAM)pStatusDone);
}

void Web::Downloader::_Worker::_BuildResponseHeader(const String *rh, Hash<String> *pHash)
{
	Array<String> lines;
	rh->explode(L"\r\n", &lines);
	pHash->removeAll();
	for(int i = 0; i < lines.size(); ++i) {
		if(!lines[i].len()) continue;
		int colonIdx = lines[i].find(L':');
		if(colonIdx == -1) { // not a key/value pair, probably response line
			(*pHash)[L""] = lines[i]; // empty key
		} else {
			String key, val;
			key.getSubstrFrom(lines[i].str(), 0, colonIdx);
			val.getSubstrFrom(lines[i].str(), colonIdx + 1).trim();
			(*pHash)[key] = val;
		}
	}
}