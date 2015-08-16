/*!
 * @file
 * @brief Regular windows, created through CreateWindowEx and using a WNDPROC.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "wnd_event.h"
#include "sys.h"

/*
       +-- TopLevel <-----------+                     +-- [WindowMain]
       |                        +-- WindowTopLevel <--+
       |                     <--+                     +-- [WindowModal]
       +-- Event <-- Window
Wnd <--+                     <--+
       |                        +-- [WindowChild]
       +-- Child <--------------+
*/

namespace wolf {
namespace wnd {

/// Base class to any regular (non-dialog) window.
class Window : virtual public EventWindow {
protected:
	ATOM _atom;
public:
	Window(ATOM atom) : _atom(atom) { }
	virtual ~Window() = 0;
	static ATOM Register(const wchar_t *className, int iconId=0, sys::Cursor cursor=sys::Cursor::ARROW, sys::Color bg=sys::Color::BUTTON);
protected:
	void    invalidateRect(bool bgErase=true);
	LRESULT defWindowProc(UINT msg, WPARAM wp, LPARAM lp);	
protected:
	static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual void _onCreate() = 0;
	virtual void _internalEvents() override;
private:
	EventWindow::_processMessage;
};


/// Intermediary class to any regular (non-dialog) window which acts as a top-level window.
class WindowTopLevel : public Window, public TopLevel {
private:
	HWND _hWndCurFocus;
protected:
	SIZE  _sz;
	HMENU _hMenu;
	DWORD _resz, _maximize, _dropFiles;
public:
	enum class Resize    { YES=WS_SIZEBOX, NO=WS_BORDER };
	enum class Maximize  { YES=WS_MAXIMIZEBOX, NO=0 };
	enum class DropFiles { YES=WS_EX_ACCEPTFILES, NO=0 };
public:
	WindowTopLevel(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d, HMENU hMenu)
		: Window(atom), _hWndCurFocus(nullptr),
			_sz(sz), _resz(static_cast<DWORD>(r)), _maximize(static_cast<DWORD>(m)),
			_dropFiles(static_cast<DWORD>(d)), _hMenu(hMenu) { }
	virtual ~WindowTopLevel() = 0;
protected:
	virtual void _onCreate() override;
	virtual void _internalEvents() override;
private:
	TopLevel::_WM_ORIGTHREAD;
};


/// Inherit from this class to create a regular (non-dialog) window to run as the default top-level program window.
class WindowMain : public WindowTopLevel {
public:
	WindowMain(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr)
		: WindowTopLevel(atom, sz, r, m, d, hMenu) { }
	virtual ~WindowMain() = 0;
	int run(HINSTANCE hInst, int cmdShow);
private:
	void _onCreate() override;
	void _internalEvents() override;
	Window::_atom;
	Window::_WindowProc;
	WindowTopLevel::_sz;
	WindowTopLevel::_hMenu;
	WindowTopLevel::_resz;
	WindowTopLevel::_maximize;
	WindowTopLevel::_dropFiles;
};


/// Inherit from this class to create a regular (non-dialog) window to be displayed as modal top-level window.
class WindowModal : public WindowTopLevel {
public:
	WindowModal(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr)
		: WindowTopLevel(atom, sz, r, m, d, hMenu) { }
	virtual ~WindowModal() = 0;
	void show(Wnd *owner);
private:
	void _onCreate() override;
	void _internalEvents() override;
	Window::_atom;
	Window::_WindowProc;
	WindowTopLevel::_sz;
	WindowTopLevel::_hMenu;
	WindowTopLevel::_resz;
	WindowTopLevel::_maximize;
	WindowTopLevel::_dropFiles;
};


/// Inherit from this class to create a regular (non-dialog) window to be used as a child of another window.
class WindowChild : public Window, public Child {
public:
	enum class Container { YES=WS_EX_CONTROLPARENT, NO=0 };
	enum class Border    { YES=WS_EX_CLIENTEDGE, NO=0 };
	enum class TabStop   { YES=(WS_TABSTOP | WS_GROUP), NO=0 };
	enum class Scroll    { VERT=WS_VSCROLL, HORZ=WS_HSCROLL, BOTH=(WS_VSCROLL | WS_HSCROLL), NONE=0 };

private:
	DWORD _container, _border, _tabStop, _scroll;
public:
	WindowChild(ATOM atom, Container c, Border b, TabStop t, Scroll s=Scroll::NONE)
		: Window(atom), _container(static_cast<DWORD>(c)), _border(static_cast<DWORD>(b)),
			_tabStop(static_cast<DWORD>(t)), _scroll(static_cast<DWORD>(s)) { }
	virtual ~WindowChild() = 0;
	void create(Wnd *parent, int ctrlId, POINT pos, SIZE sz);
protected:
	void getScrollInfo(int fnBar, SCROLLINFO& si);
	int  setScrollInfo(int fnBar, const SCROLLINFO& si, bool redraw);
private:
	void _onCreate() override;
	void _internalEvents() override;
	Child::_drawThemeBorders;
	Window::_atom;
	Window::_WindowProc;
};

}//namespace wnd
}//namespace wolf