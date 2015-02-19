
#pragma once
#include "../owl/owl.h"
using namespace owl;
using std::wstring;

struct Convert {
public:
	static bool PathsAreValid(const File::Ini& ini, wstring *pErr=nullptr);

	static bool ToWav(const File::Ini& ini, wstring src, wstring dest, bool delSrc,
		wstring *pErr=nullptr);
	
	static bool ToFlac(const File::Ini& ini, wstring src, wstring dest, bool delSrc,
		const wstring& quality, wstring *pErr=nullptr);
	
	static bool ToMp3(const File::Ini& ini, wstring src, wstring dest, bool delSrc,
		const wstring& quality, bool isVbr, wstring *pErr=nullptr);
private:
	static bool _CheckDestFolder(wstring& dest, wstring *pErr=nullptr);
	static bool _Execute(const wstring& cmdLine, const wstring& src, bool delSrc,
		wstring *pErr=nullptr);
};