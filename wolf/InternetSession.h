/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <Windows.h>
#include <winhttp.h>

namespace wolf {

class InternetSession final {
private:
	HINTERNET _hSession;
public:
	~InternetSession();
	InternetSession();
	InternetSession(InternetSession&& is);
	InternetSession& operator=(InternetSession&& is);
	HINTERNET        hSession() const;
	void             close();
	bool             init(std::wstring *pErr = nullptr, const wchar_t *userAgent = L"WOLF/1.0");

	static std::wstring formatError(DWORD lastError, const wchar_t *funcName);
};

}//namespace wolf