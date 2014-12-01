//
// Font automation.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "String.h"

class Font final {
public:
	struct Info final {
		wchar_t name[LF_FACESIZE];
		int     size;
		bool    bold;
		bool    italic;

		Info() : size(0), bold(false), italic(false) { *name = L'\0'; }
		Info(const Info& other) { operator=(other); }
		Info& operator=(const Info& other) {
			size = other.size;
			bold = other.bold;
			italic = other.italic;
			lstrcpy(name, other.name);
			return *this;
		}
	};

private:
	HFONT _hFont;
public:
	Font()                  : _hFont(nullptr) { }
	Font(HFONT hfont)       : _hFont(hfont)   { }
	Font(const Font& other) : _hFont(nullptr) { operator=(other); }
	Font(Font&& other)      : _hFont(nullptr) { operator=(MOVE(other)); }
	~Font()                 { release(); }

	Font& operator=(HFONT hfont)       { release(); _hFont = hfont; return *this; }
	Font& operator=(const Font& other) { release(); cloneFrom(other); return *this; }
	Font& operator=(Font&& other)      { _hFont = other._hFont; other.release(); return *this; }

	HFONT hFont() const            { return _hFont; }
	void  release()                { if(_hFont) { ::DeleteObject(_hFont); _hFont = nullptr; } }
	Font& create(const wchar_t *name, int size, bool bold=false, bool italic=false);
	Font& create(const Info& info) { return create(info.name, info.size, info.bold, info.italic); }
	Font& cloneFrom(const Font& font);
	Info  getInfo() const;
	Font& apply(HWND hWnd);
	Font& applyOnChildren(HWND hWnd);

	static bool Exists(const wchar_t *name);
	static Info GetDefaultDialogFontInfo();
private:
	static void _LogfontToInfo(const LOGFONT& lf, Info& info);
};