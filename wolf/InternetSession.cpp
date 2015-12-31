/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "InternetSession.h"
#include "Str.h"
#pragma comment(lib, "Winhttp.lib")
using namespace wolf;
using std::wstring;

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
			if (pErr) *pErr = formatError(GetLastError(), L"WinHttpOpen");
			return false;
		}
	}

	if (pErr) pErr->clear();
	return true;
}

wstring InternetSession::formatError(DWORD lastError, const wchar_t *funcName)
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