//
// Performs all file conversions.
// Morning and evening of Saturday, July 28, 2012.
//

#pragma once
#include "../toolow/Thread.h"
#include "../toolow/String.h"

//__________________________________________________________________________________________________
class Converter : public Thread {
public:
	enum Notification { WM_FILEDONE=WM_USER+1 };
	Converter(HWND hParent, const wchar_t *src, bool delSrc, const wchar_t *quality, bool isVbr, const wchar_t *dest);
	static bool PathsAreValid(String *pErr);
protected:
	static String* _PathOfLame(String *pBuf);
	static String* _PathOfFlac(String *pBuf);
	void _doExec(String *cmdLine);

	HWND    _hParent;
	String  _srcPath, _destPath;
	bool    _delSrc;
	wchar_t _quality[8];
	bool    _isVbr;
};

//__________________________________________________________________________________________________
class ConverterMp3 : public Converter {
public:
	ConverterMp3(HWND hParent, const wchar_t *src, bool delSrc, const wchar_t *quality, bool isVbr, const wchar_t *dest=NULL)
		: Converter(hParent, src, delSrc, quality, isVbr, dest) { }
private:
	void onRun();
};

//__________________________________________________________________________________________________
class ConverterFlac : public Converter {
public:
	ConverterFlac(HWND hParent, const wchar_t *src, bool delSrc, const wchar_t *quality, const wchar_t *dest=NULL)
		: Converter(hParent, src, delSrc, quality, false, dest) { }
private:
	void onRun();
};

//__________________________________________________________________________________________________
class ConverterWav : public Converter {
public:
	ConverterWav(HWND hParent, const wchar_t *src, bool delSrc, const wchar_t *dest=NULL)
		: Converter(hParent, src, delSrc, NULL, false, dest) { }
private:
	void onRun();
};