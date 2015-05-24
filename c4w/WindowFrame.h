/*!
 * Regular windows, created through CreateWindowEx and using a WNDPROC.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma once
#include "Sys.h"
#include "Window.h"

/*
          +-- WindowPopup <--+                 +-- [FrameApp]
          |                  +-- FramePopup <--+
          |         <--------+                 +-- [FrameModal]
          +-- Frame
Window <--+         <--------+
          |                  +-- [FrameCtrl]
          +-- WindowCtrl <---+
*/

namespace c4w {

// Base class to any regular window.
class Frame : virtual public Window {
protected:
	ATOM _atom;
public:
	Frame(ATOM atom) : _atom(atom) { }
	virtual ~Frame() = 0;
	void invalidateRect(bool bgErase=true) { ::InvalidateRect(hWnd(), 0, bgErase); }
	static ATOM Register(const wchar_t *className, int iconId=0, sys::Cursor cursor=sys::Cursor::ARROW, sys::Color bg=sys::Color::BUTTON);
protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};


// Intermediary class to any regular window which acts as a container of child windows.
class FramePopup : public Frame, public WindowPopup {
private:
	HWND _hWndCurFocus;
protected:
	SIZE   _sz;
	HACCEL _hAccelTable;
	HMENU  _hMenu;
	DWORD  _resz, _maximize, _dropFiles;
public:
	enum class Resize    { YES=WS_SIZEBOX, NO=WS_BORDER };
	enum class Maximize  { YES=WS_MAXIMIZEBOX, NO=0 };
	enum class DropFiles { YES=WS_EX_ACCEPTFILES, NO=0 };
public:
	FramePopup(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d, HMENU hMenu, HACCEL hAccelTable)
		: Frame(atom), _hWndCurFocus(nullptr),
			_sz(sz), _hAccelTable(hAccelTable), _hMenu(hMenu), _resz(static_cast<DWORD>(r)),
			_maximize(static_cast<DWORD>(m)), _dropFiles(static_cast<DWORD>(d)) { }
	virtual ~FramePopup() = 0;
protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp) override;
	Window createChild(const wchar_t *className, int id, const wchar_t *caption, DWORD exStyle, DWORD style, POINT pos, SIZE size, LPVOID lp=nullptr);
private:
	WindowPopup::_setWheelHoverBehavior;
	WindowPopup::_handleSendOrPostFunction;
};


// Base class to a regular window to run as the default program window.
class FrameApp : public FramePopup {
public:
	FrameApp(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr, HACCEL hAccelTable=nullptr)
		: FramePopup(atom, sz, r, m, d, hMenu, hAccelTable) { }
	virtual ~FrameApp() = 0;
	int run(HINSTANCE hInst, int cmdShow);
protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Frame::_WindowProc;
	Frame::_atom;
	FramePopup::_hAccelTable;
	FramePopup::_hMenu;
	FramePopup::_resz;
	FramePopup::_maximize;
	FramePopup::_dropFiles;
};


// Base class to a regular window to be displayed as modal.
class FrameModal : public FramePopup {
public:
	FrameModal(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr, HACCEL hAccelTable=nullptr)
		: FramePopup(atom, sz, r, m, d, hMenu, hAccelTable) { }
	virtual ~FrameModal() = 0;
	void show(Window *parent);
protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Frame::_WindowProc;
	Frame::_atom;
	FramePopup::_hAccelTable;
	FramePopup::_hMenu;
	FramePopup::_resz;
	FramePopup::_maximize;
	FramePopup::_dropFiles;
};


// Base class to a regular window to be used as a child of another window.
class FrameCtrl : public Frame, public WindowCtrl {
private:
	DWORD _container, _border, _tabStop, _scroll;
public:
	enum class Container { YES=WS_EX_CONTROLPARENT, NO=0 };
	enum class Border    { YES=WS_EX_CLIENTEDGE, NO=0 };
	enum class TabStop   { YES=(WS_TABSTOP | WS_GROUP), NO=0 };
	enum class Scroll    { VERT=WS_VSCROLL, HORZ=WS_HSCROLL, BOTH=(WS_VSCROLL | WS_HSCROLL), NONE=0 };
public:
	FrameCtrl(ATOM atom, Container c, Border b, TabStop t, Scroll s=Scroll::NONE)
		: Frame(atom), _container(static_cast<DWORD>(c)), _border(static_cast<DWORD>(b)),
			_tabStop(static_cast<DWORD>(t)), _scroll(static_cast<DWORD>(s)) { }
	virtual ~FrameCtrl() = 0;
	void create(Window *parent, int ctrlId, POINT pos, SIZE sz);
	void getScrollInfo(int fnBar, SCROLLINFO *psi)              { ::GetScrollInfo(hWnd(), fnBar, psi); }
	int  setScrollInfo(int fnBar, SCROLLINFO *psi, bool redraw) { return ::SetScrollInfo(hWnd(), fnBar, psi, static_cast<BOOL>(redraw)); }
protected:
	virtual LRESULT wndProc(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	WindowCtrl::_drawBorders;
	Frame::_atom;
};

}//namespace c4w