/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "InternetSession.h"

namespace wolf {

class InternetUrl final {
public:
	InternetUrl();
	bool           crack(const wchar_t *address, std::wstring *pErr=nullptr);
	bool           crack(const std::wstring& address, std::wstring *pErr=nullptr);
	const wchar_t* scheme() const;
	const wchar_t* host() const;
	const wchar_t* user() const;
	const wchar_t* pwd() const;
	const wchar_t* path() const;
	const wchar_t* extra() const;
	std::wstring   pathAndExtra() const;
	int            port() const;
	bool           isHttps() const;
private:
	URL_COMPONENTS _uc;
	wchar_t        _scheme[16], _host[64], _user[64], _pwd[64], _path[256], _extra[256];
};

}//namespace wolf