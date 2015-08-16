/*!
 * @file
 * @brief Assorted resources.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <vector>
#include "sys.h"

namespace wolf {
namespace res {

/// %Date and time operations.
class Date final {
private:
	SYSTEMTIME _st;
public:
	Date()                              { setNow(); }
	explicit Date(LONGLONG ms)          { setFromMs(ms); }
	explicit Date(const SYSTEMTIME& st) { setFromSt(st); }
	explicit Date(const FILETIME& ft)   { setFromFt(ft); }
	Date& setNow();
	Date& setFromSt(const SYSTEMTIME& st) { ::memcpy(&_st, &st, sizeof(SYSTEMTIME)); return *this; }
	Date& setFromMs(LONGLONG ms);
	Date& setFromFt(const FILETIME& ft);
	const SYSTEMTIME& get() const { return _st; }
	LONGLONG          getTimestamp() const;
	LONGLONG          minus(const Date& other) const;
	Date&             addMs(LONGLONG ms);
	Date&             addSec(LONGLONG sec) { return addMs(sec * 1000); }
	Date&             addMin(LONGLONG min) { return addSec(min * 60); }
	Date&             addHour(LONGLONG h)  { return addMin(h * 60); }
	Date&             addDay(LONGLONG d)   { return addHour(d * 24); }
private:
	static void _StToLi(const SYSTEMTIME& st, LARGE_INTEGER& li);
	static void _LiToSt(const LARGE_INTEGER& li, SYSTEMTIME& st);
};


/// HMENU wrapper.
class Menu {
protected:
	HMENU _hMenu;
public:
	Menu()            : _hMenu(nullptr) { }
	Menu(HMENU hMenu) : _hMenu(hMenu) { }
	virtual ~Menu()   { }
	
	HMENU hMenu() const             { return _hMenu; }
	int   size() const              { return ::GetMenuItemCount(_hMenu); }
	void  destroy()                 { if (_hMenu) { ::DestroyMenu(_hMenu); _hMenu = nullptr; } }
	Menu  getSubmenu(int pos) const { return Menu(::GetSubMenu(_hMenu, pos)); }
	WORD  getCmdId(int pos) const   { return ::GetMenuItemID(_hMenu, pos); }
	Menu& addSeparator();
	Menu& addItem(const wchar_t *caption, WORD cmdId);
	Menu& addItem(const std::wstring& caption, WORD cmdId) { return addItem(caption.c_str(), cmdId); }
	Menu& enableItem(std::initializer_list<WORD> cmdIds, bool doEnable);
	Menu  addSubmenu(const wchar_t *caption);
	Menu  addSubmenu(const std::wstring& caption)          { return addSubmenu(caption.c_str()); }
protected:
	virtual void _createIfNotYet() { }
};

/// A top-level window main menu.
class MenuMain final : public Menu {
private:
	void _createIfNotYet() { if (!_hMenu) _hMenu = CreateMenu(); }
};

/// A context menu popup.
class MenuContext final : public Menu {
public:
	~MenuContext()         { destroy(); }
	void popAtPoint(HWND hParent, POINT pt, HWND hWndCoordsRelativeTo=nullptr);
private:
	void _createIfNotYet() { if (!_hMenu) _hMenu = CreatePopupMenu(); }
};


/// Accelerator table automation.
class AccelTable final {
public:
	enum class Type { CHAR=0, VIRT=TRUE };
	enum class Mod { NONE=0,
		CTRL=FCONTROL, SHIFT=FSHIFT, ALT=FALT,
		CTRLSHIFTALT=(FCONTROL|FSHIFT|FALT),
		CTRLSHIFT=(FCONTROL|FSHIFT),
		CTRLALT=(FCONTROL|FALT),
		SHIFTALT=(FSHIFT|FALT)
	};
	struct Key {
		Type type;
		WORD key, cmdId;
		Mod  modifiers;
	};

private:
	HACCEL _hAccel;
public:
	AccelTable()  : _hAccel(nullptr) { }
	~AccelTable() { destroy(); }

	HACCEL hAccel() const { return _hAccel; }
	void   destroy();
	void   add(std::initializer_list<Key> keys);
};


/// HFONT wrapper.
class Font final {
public:
	/// %Font information.
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
	Font(const Font& other) : Font() { operator=(other); }
	Font(Font&& other)      : Font() { operator=(std::move(other)); }
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


/// HICON wrapper.
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


/// Device context automation.
class DC {
public:
	/// HPEN wrapper.
	class Pen {
	private:
		HPEN _hPen;
	public:
		enum class Style { SOLID=PS_SOLID, DASH=PS_DASH, DOT=PS_DOT, DASHDOT=PS_DASHDOT, DASHDOTDOT=PS_DASHDOTDOT };
		Pen(Style style, int width, COLORREF color) : _hPen(::CreatePen(static_cast<int>(style), width, color)) { }
		~Pen()                                      { ::DeleteObject(_hPen); }
		HPEN hPen() const                           { return _hPen; }
	};

	/// HBRUSH wrapper.
	class Brush {
	private:
		HBRUSH _hBrush;
	public:
		enum class Pattern { BDIAGONAL=HS_BDIAGONAL, CROSS=HS_CROSS, DIAGCROSS=HS_DIAGCROSS, FDIAGONAL=HS_FDIAGONAL, HORIZONTAL=HS_HORIZONTAL, VERTICAL=HS_VERTICAL };
		Brush(COLORREF color)                : _hBrush(::CreateSolidBrush(color)) { }
		Brush(Pattern hatch, COLORREF color) : _hBrush(::CreateHatchBrush(static_cast<int>(hatch), color)) { }
		Brush(sys::Color color)              : _hBrush(::GetSysColorBrush(static_cast<int>(color))) { }
		~Brush()                             { ::DeleteObject(_hBrush); }
		HBRUSH hBrush() const                { return _hBrush; }
	};

public:
	int cx, cy;
protected:
	HDC  _hdc;
	HWND _hWnd;
public:
	explicit DC(HWND hwnd, HDC hDC=nullptr);
	virtual ~DC() { }

	HDC      hdc() const                { return _hdc; }
	HWND     hWnd() const               { return _hWnd; }
	DC&      deleteObject(HGDIOBJ obj)  { ::DeleteObject(obj); return *this; }
	HGDIOBJ  selectObject(HGDIOBJ obj)  { return ::SelectObject(_hdc, obj); }
	HGDIOBJ  selectStockFont()          { return selectObject(::GetStockObject(SYSTEM_FONT)); }
	HGDIOBJ  selectStockPen()           { return selectObject(::GetStockObject(BLACK_PEN)); }
	HGDIOBJ  selectStockBrush()         { return selectObject(::GetStockObject(WHITE_BRUSH)); }
	DC&      moveTo(int x, int y)       { ::MoveToEx(_hdc, x, y, nullptr); return *this; }
	DC&      lineTo(int x, int y)       { ::LineTo(_hdc, x, y); return *this; }
	DC&      lineRect(int left, int top, int right, int bottom) { return moveTo(left, top).lineTo(right, top).lineTo(right, bottom).lineTo(left, bottom); }
	DC&      lineRect(const RECT& rc)                           { return lineRect(rc.left, rc.top, rc.right, rc.bottom); }
	DC&      setBkTransparent(bool yes) { ::SetBkMode(_hdc, yes ? TRANSPARENT : OPAQUE); return *this; }
	DC&      setBkColor(COLORREF color=-1);
	COLORREF getBkBrushColor();
	DC&      setTextColor(COLORREF color)                             { ::SetTextColor(_hdc, color); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text, int numChars) { ::TextOut(_hdc, x, y, text, numChars); return *this; }
	DC&      textOut(int x, int y, const wchar_t *text)               { return textOut(x, y, text, ::lstrlen(text)); }
	DC&      textOut(int x, int y, const std::wstring& text)          { return textOut(x, y, text.c_str(), static_cast<int>(text.length())); }
	DC&      drawText(int x, int y, int cx, int cy, const wchar_t *text, UINT fmtFlags=0);
	DC&      drawText(int x, int y, int cx, int cy, const std::wstring& text, UINT fmtFlags=0) { return drawText(x, y, cx, cy, text.c_str(), fmtFlags); }
	SIZE     getTextExtent(const wchar_t *text, int numChars)                        { SIZE sz; ::GetTextExtentPoint32(_hdc, text, numChars, &sz); return sz; }
	SIZE     getTextExtent(const wchar_t *text)                                      { return getTextExtent(text, ::lstrlen(text)); }
	SIZE     getTextExtent(const std::wstring& text)                                 { return getTextExtent(text.c_str(), static_cast<int>(text.length())); }
	DC&      fillRect(int left, int top, int right, int bottom, HBRUSH hBrush)       { RECT rc = { left, top, right, bottom }; ::FillRect(_hdc, &rc, hBrush); return *this; }
	DC&      fillRgn(HRGN hrgn, HBRUSH hBrush)                                       { ::FillRgn(_hdc, hrgn, hBrush); return *this; }
	DC&      polygon(const POINT *points, int numPoints)                             { ::Polygon(_hdc, points, numPoints); return *this; }
	DC&      polygon(int left, int top, int right, int bottom);
	DC&      drawEdge(const RECT& rc, int edge, int flags)                           { ::DrawEdge(_hdc, const_cast<RECT*>(&rc), edge, flags); return *this; }
	DC&      drawEdge(int left, int top, int right, int bottom, int edge, int flags) { RECT rc = { left, top, right, bottom }; return drawEdge(rc, edge, flags); }
};

/// Device context for within WM_PAINT, uses BeginPaint/EndPaint.
class DCSimple : public DC {
protected:
	PAINTSTRUCT _ps;
public:
	explicit DCSimple(HWND hwnd) : DC(hwnd, ::BeginPaint(hwnd, &_ps)) { }
	virtual ~DCSimple()          { ::EndPaint(_hWnd, &_ps); }
};

/// Device contex for within WM_PAINT, double-buffered; you must return zero on WM_ERASEBKGND message.
class DCBuffered : public DCSimple {
protected:
	HBITMAP _hBmp, _hBmpOld;
public:
	explicit DCBuffered(HWND hwnd);
	virtual ~DCBuffered();
};

}//namespace res
}//namespace wolf