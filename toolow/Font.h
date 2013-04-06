//
// Font automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include <Windows.h>

class Font {
public:
	struct Info {
		wchar_t name[LF_FACESIZE];
		int     size;
		bool    bold;
		bool    italic;
	};

public:
	Font()  : _hFont(0) { }
	~Font() { release(); }

	HFONT hFont() const             { return _hFont; }
	void  release()                 { if(_hFont) { ::DeleteObject(_hFont); _hFont = 0; } }
	Font& create(const wchar_t *name, int size, bool bold=false, bool italic=false);
	Font& create(const Info *pInfo) { return create(pInfo->name, pInfo->size, pInfo->bold, pInfo->italic); }
	Font& cloneFrom(const Font *pFont);
	Info* getInfo(Info *pInfo) const;
	Font& apply(HWND hWnd);
	Font& applyOnChildren(HWND hWnd);

	static bool Exists(const wchar_t *name);
	static void GetDefaultDialogFontInfo(Info *pInfo);
	
private:
	HFONT _hFont;

	static void _LogfontToInfo(const LOGFONT *lf, Info *pInfo);
	static BOOL CALLBACK _ApplyOnChild(HWND hWnd, LPARAM lp);
	static int  CALLBACK _EnumFontFamProc(ENUMLOGFONT *lpelf, NEWTEXTMETRIC *lpntm, DWORD FontType, LPARAM lp);
};