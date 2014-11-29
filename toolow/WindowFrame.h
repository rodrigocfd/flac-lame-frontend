//
// Realm of the regular windows, those who take a WNDPROC as a procedure and are created through CreateWindowEx().
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "System.h"
#include "Window.h"

//__________________________________________________________________________________________________
// Base class to any regular window.
//
class Frame : virtual public Window {
public:
	virtual ~Frame() = 0;
	void invalidateRect(bool bgErase=true) { ::InvalidateRect(hWnd(), 0, bgErase); }
	static ATOM Register(const wchar_t *className, System::Cursor cursor=System::Cursor::ARROW, int iconId=0, System::Color bg=System::Color::BUTTON);
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp);
	static LRESULT CALLBACK _WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};

//__________________________________________________________________________________________________
// Intermediary class to any regular window which acts as a container of child windows.
//
class FramePopup : public Frame, public WindowPopup {
public:
	struct Style {
		enum Maximize { NOMAXIMIZABLE=0, MAXIMIZABLE=WS_MAXIMIZEBOX };
		enum Resize   { NORESIZABLE=WS_BORDER, RESIZABLE=WS_SIZEBOX };
		enum Drop     { NODROPPABLE=0, DROPPABLE=WS_EX_ACCEPTFILES };
	};
private:
	HWND _hWndCurFocus;
public:
	virtual ~FramePopup() = 0;
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp) override;
	Window createButton(const wchar_t *caption, int id, int x, int y, int cx, bool def=false);
	Window createLabel(const wchar_t *caption, int x, int y, int cx, int id=0);
	Window createCheck(const wchar_t *caption, int id, int x, int y, int cx);
	Window createEdit(int id, int x, int y, int cx, UINT extraStyles=0);
	Window createCombo(int id, int x, int y, int cx);
private:
	WindowPopup::_setWheelHoverBehavior;
	WindowPopup::_handleSendOrPostFunction;
};

//__________________________________________________________________________________________________
// Base class to a regular window to run as the default program window.
//
class FrameApp : public FramePopup {
public:
	virtual ~FrameApp() = 0;
	virtual int run(HINSTANCE hInst, int cmdShow, ATOM atom, const wchar_t *caption, int cxClient, int cyClient,
		FramePopup::Style::Maximize maximizable, FramePopup::Style::Resize resizable,
		FramePopup::Style::Drop droppable, HMENU hMenu=0, HACCEL hAccel=0);
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Frame::_WindowProc;
};

//__________________________________________________________________________________________________
// Base class to a regular window to be displayed as modal.
//
class FrameModal : public FramePopup {
public:
	virtual ~FrameModal() = 0;
	virtual int show(Window *parent, ATOM atom, const wchar_t *caption, int cxClient, int cyClient,
		FramePopup::Style::Resize resizable, HACCEL hAccel=0);
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	Frame::_WindowProc;
};

//__________________________________________________________________________________________________
// Base class to a regular window to be used as a child of another window.
//
class FrameCtrl : public Frame, public WindowCtrl {
public:
	struct Style {
		enum Border    { NOBORDER=0, BORDER=WS_EX_CLIENTEDGE };
		enum Container { NOCONTAINER=0, CONTAINER=WS_EX_CONTROLPARENT };
		enum Scroll    { NOSCROLL=0, HSCROLL=WS_HSCROLL, VSCROLL=WS_VSCROLL, BOTHSCROLL=(WS_HSCROLL|WS_VSCROLL) };
		enum TabStop   { NOTABSTOP=0, TABSTOP=(WS_TABSTOP|WS_GROUP) };
	};
public:
	virtual ~FrameCtrl() = 0;
	virtual void create(int id, Window *parent, ATOM atom, int x, int y, int cx, int cy,
		Style::Border border, Style::Container container, Style::Scroll scroll, Style::TabStop tabStop);
	void getScrollInfo(int fnBar, SCROLLINFO *psi)              { ::GetScrollInfo(hWnd(), fnBar, psi); }
	int  setScrollInfo(int fnBar, SCROLLINFO *psi, bool redraw) { return ::SetScrollInfo(hWnd(), fnBar, psi, (BOOL)redraw); }
protected:
	virtual LRESULT msgHandler(UINT msg, WPARAM wp, LPARAM lp) override;
private:
	WindowCtrl::_drawBorders;
};