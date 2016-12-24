
#pragma once
#include <string>
#include "../wet/file_ini.h"

struct Convert final {
public:
	static bool paths_are_valid(
		const wet::file_ini& ini,
		std::wstring*        pErr = nullptr);

	static bool to_wav(
		const wet::file_ini& ini,
		std::wstring         src,
		std::wstring         dest,
		bool                 delSrc,
		std::wstring*        pErr = nullptr);
	
	static bool to_flac(
		const wet::file_ini& ini,
		std::wstring         src,
		std::wstring         dest,
		bool                 delSrc,
		const std::wstring&  quality,
		std::wstring*        pErr = nullptr);
	
	static bool to_mp3(
		const wet::file_ini& ini,
		std::wstring         src,
		std::wstring         dest,
		bool                 delSrc,
		const std::wstring&  quality,
		bool                 isVbr,
		std::wstring*        pErr = nullptr);
private:
	static bool _check_dest_folder(
		std::wstring&        dest,
		std::wstring*        pErr = nullptr);

	static bool _execute(
		const std::wstring&  cmdLine,
		const std::wstring&  src,
		bool                 delSrc,
		std::wstring*        pErr = nullptr);
};