
#pragma once
#include "std.h"
#include <winlamb/file_ini.h>

struct Convert final {
private:
	Convert() = delete;

public:
	static void validate_paths(const wl::file_ini& ini);
	static void to_wav(const wl::file_ini& ini, wstring src, wstring dest, bool delSrc);
	static void to_flac(const wl::file_ini& ini, wstring src, wstring dest,
		bool delSrc, const wstring& quality);
	static void to_mp3(const wl::file_ini& ini, wstring src, wstring dest,
		bool delSrc, const wstring& quality, bool isVbr);

private:
	static void _validate_dest_folder(wstring& dest);
	static void _execute(const wstring& cmdLine, const wstring& src, bool delSrc);
	static void _raw_execute(wstring cmdLine);
};