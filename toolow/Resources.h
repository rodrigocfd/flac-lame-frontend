//
// Automation for some resources.
// Part of WOLF - Win32 Object Lambda Framework.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/wolf
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
	void  release()                { if (_hFont) { ::DeleteObject(_hFont); _hFont = nullptr; } }
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


class Menu {
private:
	HMENU _hMenu;
public:
	Menu()            : _hMenu(nullptr) { }
	Menu(HMENU hMenu) : _hMenu(hMenu) { }
	
	HMENU hMenu() const             { return _hMenu; }
	int   size() const              { return ::GetMenuItemCount(_hMenu); }
	void  destroy()                 { if (_hMenu) { ::DestroyMenu(_hMenu); _hMenu = nullptr; } }
	Menu  getSubmenu(int pos) const { return Menu(::GetSubMenu(_hMenu, pos)); }
	WORD  getCmdId(int pos) const   { return ::GetMenuItemID(_hMenu, pos); }
	Menu& createMain(HWND owner);
	Menu& createPopup();
	Menu& appendSeparator();
	Menu& appendItem(const wchar_t *caption, WORD cmdId);
	Menu& enableItem(initializer_list<WORD> cmdIds, bool doEnable);
	Menu  appendSubmenu(const wchar_t *caption);
private:
	void _checkDummyEntry();
};


class Icon final {
private:
	HICON _hIcon;
public:
	Icon()  : _hIcon(nullptr) { }
	~Icon() { this->free(); }

	HICON hIcon() const          { return _hIcon; }
	Icon& free()                 { if (_hIcon) ::DestroyIcon(_hIcon); return *this; }
	Icon& operator=(HICON hIcon) { _hIcon = hIcon; return *this; }
	Icon& getFromExplorer(const wchar_t *fileExtension);
	Icon& getFromResource(int iconId, int size, HINSTANCE hInst=nullptr);

	static void IconToLabel(HWND hStatic, int idIconRes, BYTE size);
};