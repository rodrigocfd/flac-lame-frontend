
#pragma once
#include <winlamb/file_ini.h>

struct Convert final {
private:
	Convert() = delete;

public:
	static void validatePaths(const wl::file_ini& ini);
	static void toWav(const wl::file_ini& ini, std::wstring src, std::wstring dest, bool delSrc);
	static void toFlac(const wl::file_ini& ini, std::wstring src, std::wstring dest,
		bool delSrc, const std::wstring& quality);
	static void toMp3(const wl::file_ini& ini, std::wstring src, std::wstring dest,
		bool delSrc, const std::wstring& quality, bool isVbr);

private:
	static void _validateDestFolder(std::wstring& dest);
	static void _execute(const std::wstring& cmdLine, const std::wstring& src, bool delSrc);
};