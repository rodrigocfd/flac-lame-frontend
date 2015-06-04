/*!
 * @file
 * @brief Regular windows, created through CreateWindowEx and using a WNDPROC.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowEvent.h"
#include "Sys.h"

/*
          +-- WindowTopLevel <---------------+                    +-- [FrameApp]
          |                                  +-- FrameTopLevel <--+
          |                         <--------+                    +-- [FrameModal]
          +-- WindowEvent <-- Frame
Window <--+                         <--------+
          |                                  +-- [FrameChild]
          +-- WindowChild <------------------+
*/

namespace wolf {

/// Base class to any regular (non-dialog) window.
class Frame : virtual public WindowEventFrame {
protected:
	ATOM _atom;
public:
	Frame(ATOM atom) : _atom(atom) { }
	virtual ~Frame() = 0;
	static ATOM Register(const wchar_t *className, int iconId=0, sys::Cursor cursor=sys::Cursor::ARROW, sys::Color bg=sys::Color::BUTTON);
protected:
	void    invalidateRect(bool bgErase=true);
	LRESULT defWindowProc(UINT msg, WPARAM wp, LPARAM lp);	
protected:
	static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
	virtual void _onCreate() = 0;
	virtual void _internalEvents() override;
private:
	WindowEventFrame::_processMessage;
};


/// Intermediary class to any regular (non-dialog) window which acts as a top-level window.
class FrameTopLevel : public Frame, public WindowTopLevel {
private:
	HWND _hWndCurFocus;
protected:
	SIZE   _sz;
	HMENU  _hMenu;
	DWORD  _resz, _maximize, _dropFiles;
public:
	enum class Resize    { YES=WS_SIZEBOX, NO=WS_BORDER };
	enum class Maximize  { YES=WS_MAXIMIZEBOX, NO=0 };
	enum class DropFiles { YES=WS_EX_ACCEPTFILES, NO=0 };
public:
	FrameTopLevel(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d, HMENU hMenu)
		: Frame(atom), _hWndCurFocus(nullptr),
			_sz(sz), _resz(static_cast<DWORD>(r)), _maximize(static_cast<DWORD>(m)),
			_dropFiles(static_cast<DWORD>(d)), _hMenu(hMenu) { }
	virtual ~FrameTopLevel() = 0;
protected:
	Window createChild(const wchar_t *className, int id, const wchar_t *caption, DWORD exStyle, DWORD style, POINT pos, SIZE size, LPVOID lp=nullptr);
protected:
	virtual void _onCreate() override;
	virtual void _internalEvents() override;
private:
	WindowTopLevel::_WM_ORIGTHREAD;
};


/// Inherit from this class to create a regular (non-dialog) window to run as the default top-level program window.
class FrameApp : public FrameTopLevel {
public:
	FrameApp(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr)
		: FrameTopLevel(atom, sz, r, m, d, hMenu) { }
	virtual ~FrameApp() = 0;
	int run(HINSTANCE hInst, int cmdShow);
private:
	void _onCreate() override;
	void _internalEvents() override;
	Frame::_atom;
	Frame::_WindowProc;
	FrameTopLevel::_sz;
	FrameTopLevel::_hMenu;
	FrameTopLevel::_resz;
	FrameTopLevel::_maximize;
	FrameTopLevel::_dropFiles;
};


/// Inherit from this class to create a regular (non-dialog) window to be displayed as modal top-level window.
class FrameModal : public FrameTopLevel {
public:
	FrameModal(ATOM atom, SIZE sz, Resize r, Maximize m, DropFiles d=DropFiles::NO, HMENU hMenu=nullptr)
		: FrameTopLevel(atom, sz, r, m, d, hMenu) { }
	virtual ~FrameModal() = 0;
	void show(Window *owner);
private:
	void _onCreate() override;
	void _internalEvents() override;
	Frame::_atom;
	Frame::_WindowProc;
	FrameTopLevel::_sz;
	FrameTopLevel::_hMenu;
	FrameTopLevel::_resz;
	FrameTopLevel::_maximize;
	FrameTopLevel::_dropFiles;
};


/// Inherit from this class to create a regular (non-dialog) window to be used as a child of another window.
class FrameChild : public Frame, public WindowChild {
public:
	enum class Container { YES=WS_EX_CONTROLPARENT, NO=0 };
	enum class Border    { YES=WS_EX_CLIENTEDGE, NO=0 };
	enum class TabStop   { YES=(WS_TABSTOP | WS_GROUP), NO=0 };
	enum class Scroll    { VERT=WS_VSCROLL, HORZ=WS_HSCROLL, BOTH=(WS_VSCROLL | WS_HSCROLL), NONE=0 };

private:
	DWORD _container, _border, _tabStop, _scroll;
public:
	FrameChild(ATOM atom, Container c, Border b, TabStop t, Scroll s=Scroll::NONE)
		: Frame(atom), _container(static_cast<DWORD>(c)), _border(static_cast<DWORD>(b)),
			_tabStop(static_cast<DWORD>(t)), _scroll(static_cast<DWORD>(s)) { }
	virtual ~FrameChild() = 0;
	void create(Window *parent, int ctrlId, POINT pos, SIZE sz);
protected:
	void getScrollInfo(int fnBar, SCROLLINFO& si);
	int  setScrollInfo(int fnBar, const SCROLLINFO& si, bool redraw);
private:
	void _onCreate() override;
	void _internalEvents() override;
	WindowChild::_drawThemeBorders;
	Frame::_atom;
	Frame::_WindowProc;
};

}//namespace wolf