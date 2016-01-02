/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <Windows.h>

namespace wolf {

class Font final {
public:
	struct Info final {
		wchar_t name[LF_FACESIZE];
		int     size;
		bool    bold;
		bool    italic;
		Info();
		Info(const Info& other);
		Info& operator=(const Info& other);
	};

private:
	HFONT _hFont;
public:
	~Font();
	Font();
	Font(Font&& f);
	Font& operator=(HFONT hfont);
	Font& operator=(Font&& f);
	HFONT hFont() const;
	void  release();
	Font& create(const wchar_t *name, int size, bool bold = false, bool italic = false);
	Font& create(const Info& info);
	Font& cloneFrom(const Font& font);
	Info  getInfo() const;
	Font& apply(HWND hWnd);
	Font& applyOnChildren(HWND hWnd);

	static bool exists(const wchar_t *name);
	static Info getDefaultDialogFontInfo();
private:
	static void _logfontToInfo(const LOGFONT& lf, Info& info);
};

}//namespace wolf