
#pragma once
#include <winlamb/file_ini.h>

struct Convert final {
private:
	Convert() = delete;

public:
	static void validate_paths(const wl::file_ini& ini);
	static void to_wav(const wl::file_ini& ini, std::wstring src, std::wstring dest, bool delSrc);
	static void to_flac(const wl::file_ini& ini, std::wstring src, std::wstring dest,
		bool delSrc, const std::wstring& quality);
	static void to_mp3(const wl::file_ini& ini, std::wstring src, std::wstring dest,
		bool delSrc, const std::wstring& quality, bool isVbr);

private:
	static void _validate_dest_folder(std::wstring& dest);
	static void _execute(const std::wstring& cmdLine, const std::wstring& src, bool delSrc);
};