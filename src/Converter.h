//
// Performs all file conversions.
// Morning and evening of Saturday, July 28, 2012.
//

#pragma once
#include "../toolow/File.h"
#include "../toolow/Thread.h"

//__________________________________________________________________________________________________
class Converter : public Thread {
public:
	enum Notification { WM_FILEDONE=WM_USER+1 };
	Converter(HWND hParent, File::Ini *pIni, const wchar_t *src, bool delSrc, const wchar_t *quality, bool isVbr, const wchar_t *dest);
	static bool PathsAreValid(File::Ini *pIni, String *pErr);
protected:
	void _doExec(String *cmdLine);

	HWND       _hParent;
	File::Ini *_pIni;
	String     _srcPath, _destPath;
	bool       _delSrc;
	wchar_t    _quality[8];
	bool       _isVbr;
};

//__________________________________________________________________________________________________
class ConverterMp3 : public Converter {
public:
	ConverterMp3(HWND hParent, File::Ini *pIni, const wchar_t *src, bool delSrc, const wchar_t *quality, bool isVbr, const wchar_t *dest=NULL)
		: Converter(hParent, pIni, src, delSrc, quality, isVbr, dest) { }
private:
	void onRun();
};

//__________________________________________________________________________________________________
class ConverterFlac : public Converter {
public:
	ConverterFlac(HWND hParent, File::Ini *pIni, const wchar_t *src, bool delSrc, const wchar_t *quality, const wchar_t *dest=NULL)
		: Converter(hParent, pIni, src, delSrc, quality, false, dest) { }
private:
	void onRun();
};

//__________________________________________________________________________________________________
class ConverterWav : public Converter {
public:
	ConverterWav(HWND hParent, File::Ini *pIni, const wchar_t *src, bool delSrc, const wchar_t *dest=NULL)
		: Converter(hParent, pIni, src, delSrc, NULL, false, dest) { }
private:
	void onRun();
};