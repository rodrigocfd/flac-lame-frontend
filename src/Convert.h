
#pragma once
#include <string>
#include "../winutil/file_ini.h"

struct Convert final {
public:
	static bool pathsAreValid(const winutil::file_ini& ini, std::wstring *pErr = nullptr);

	static bool toWav(const winutil::file_ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		std::wstring *pErr = nullptr);
	
	static bool toFlac(const winutil::file_ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, std::wstring *pErr = nullptr);
	
	static bool toMp3(const winutil::file_ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, bool isVbr, std::wstring *pErr = nullptr);
private:
	static bool _checkDestFolder(std::wstring& dest, std::wstring *pErr = nullptr);
	static bool _execute(const std::wstring& cmdLine, const std::wstring& src, bool delSrc,
		std::wstring *pErr = nullptr);
};