/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "InternetUrl.h"
#pragma comment(lib, "Winhttp.lib")
using namespace wolf;
using std::wstring;

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
		if (pErr) *pErr = InternetSession::formatError(GetLastError(), L"WinHttpCrackUrl");
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