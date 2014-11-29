
#pragma once
#include "../toolow/toolow.h"

class Convert {
public:
	static bool PathsAreValid(const File::Ini& ini, String *pErr=nullptr);

	static bool ToWav(const File::Ini& ini, String src, String dest, bool delSrc,
		String *pErr=nullptr);
	
	static bool ToFlac(const File::Ini& ini, String src, String dest, bool delSrc,
		const String& quality, String *pErr=nullptr);
	
	static bool ToMp3(const File::Ini& ini, String src, String dest, bool delSrc,
		const String& quality, bool isVbr, String *pErr=nullptr);

private:
	static bool _CheckDestFolder(String& dest, String *pErr=nullptr);
	static bool _Execute(const String& cmdLine, const String& src, bool delSrc, String *pErr=nullptr);
};