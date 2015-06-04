
#pragma once
#include "../wolf/wolf.h"

struct Convert final {
public:
	static bool PathsAreValid(const wolf::file::Ini& ini, std::wstring *pErr=nullptr);

	static bool ToWav(const wolf::file::Ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		std::wstring *pErr=nullptr);
	
	static bool ToFlac(const wolf::file::Ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, std::wstring *pErr=nullptr);
	
	static bool ToMp3(const wolf::file::Ini& ini, std::wstring src, std::wstring dest, bool delSrc,
		const std::wstring& quality, bool isVbr, std::wstring *pErr=nullptr);
private:
	static bool _CheckDestFolder(std::wstring& dest, std::wstring *pErr=nullptr);
	static bool _Execute(const std::wstring& cmdLine, const std::wstring& src, bool delSrc,
		std::wstring *pErr=nullptr);
};