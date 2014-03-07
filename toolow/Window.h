//
// HWND wrapper.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
//

#pragma once
#include "String.h"

//__________________________________________________________________________________________________
// Base HWND wrapper.
//
class Window {
public:
	Window()                    : _hWnd(0) { }
	Window(HWND hwnd)           : _hWnd(hwnd) { }
	Window(const Window& other) : _hWnd(other._hWnd) { }

	HWND      hWnd() const                                      { return _hWnd; }
	Window&   operator=(HWND hwnd)                              { _hWnd = hwnd; return *this; }
	Window&   operator=(const Window& other)                    { _hWnd = other._hWnd; return *this; }
	Window    getParent() const                                 { return ::GetParent(_hWnd); }
	HINSTANCE getInstance() const                               { return (HINSTANCE)::GetWindowLongPtr(_hWnd, GWLP_HINSTANCE); }
	int       getId() const                                     { return ::GetDlgCtrlID(_hWnd); }
	LRESULT   sendMessage(UINT msg, WPARAM wp, LPARAM lp) const { return ::SendMessage(_hWnd, msg, wp, lp); }
	LRESULT   postMessage(UINT msg, WPARAM wp, LPARAM lp) const { return ::PostMessage(_hWnd, msg, wp, lp); }
	Window&   setText(const wchar_t *text)                      { ::SetWindowText(_hWnd, text); return *this; }
	wchar_t*  getText(wchar_t *pBuf, int szBuf) const           { ::GetWindowText(_hWnd, pBuf, szBuf); return pBuf; }
	String*   getText(String *pBuf) const                       { pBuf->reserve(::GetWindowTextLength(_hWnd)); ::GetWindowText(_hWnd, pBuf->ptrAt(0), pBuf->reserved() + 1); return pBuf; }
	String    getText() const                                   { String ret; getText(&ret); return ret; }
	Window&   setPos(HWND hInsertAfter, int x, int y, int cx, int cy, UINT flags) { ::SetWindowPos(_hWnd, hInsertAfter, x, y, cx, cy, flags); return *this; }
	Window&   setFocus()                                        { ::SetFocus(_hWnd); return *this; }
	bool      isFocused() const                                 { return ::GetFocus() == _hWnd; }
	Window&   setVisible(bool visible)                          { ::ShowWindow(_hWnd, visible ? SW_SHOW : SW_HIDE); return *this; }
	bool      isVisible() const                                 { return ::IsWindowVisible(_hWnd) == TRUE; }
	Window&   setEnable(bool doEnable)                          { ::EnableWindow(_hWnd, doEnable); return *this; }
	bool      isEnabled() const                                 { return ::IsWindowEnabled(_hWnd) == TRUE; }
	Window&   screenToClient(POINT *pPt)                        { ::ScreenToClient(_hWnd, pPt); return *this; }
	Window&   screenToClient(RECT *pRc)                         { screenToClient((POINT*)&pRc->left); return screenToClient((POINT*)&pRc->right); }
	Window&   clientToScreen(POINT *pPt)                        { ::ClientToScreen(_hWnd, pPt); return *this; }
	Window&   clientToScreen(RECT *pRc)                         { clientToScreen((POINT*)&pRc->left); return clientToScreen((POINT*)&pRc->right); }
	BOOL      getClientRect(RECT *pRc) const                    { return ::GetClientRect(_hWnd, pRc); }
	BOOL      getWindowRect(RECT *pRc) const                    { return ::GetWindowRect(_hWnd, pRc); }
private:
	HWND _hWnd;
};

//__________________________________________________________________________________________________
// Mix-in class to any popup window.
// To remove icon from a resizing modal, use EXSTYLE WS_EX_DLGMODALFRAME.
//
class WindowPopup : virtual public Window {
public:
	virtual ~WindowPopup() = 0;
protected:
	Window getChild(int id)        { return Window(::GetDlgItem(hWnd(), id)); }
	int    messageBox(const wchar_t *caption, const wchar_t *body, UINT uType=0);
	bool   getFileOpen(const wchar_t *formattedFilter, String *pBuf);
	bool   getFileOpen(const wchar_t *formattedFilter, Array<String> *pBuf);
	bool   getFileSave(const wchar_t *formattedFilter, String *pBuf, const wchar_t *defFile=0);
	bool   getFolderChoose(String *pBuf);
	void   setXButton(bool enable);
	bool   isMinimized()           { return ::IsIconic(hWnd()) == TRUE; }
	bool   isMaximized()           { return ::IsZoomed(hWnd()) == TRUE; }
	Array<String> getDroppedFiles(HDROP hDrop);
	void   setWheelHoverBehavior() { ::EnumChildWindows(hWnd(), _WheelHoverApply, 0); }
private:
	static BOOL    CALLBACK _WheelHoverApply(HWND hChild, LPARAM lp);
	static LRESULT CALLBACK _WheelHoverProc(HWND hChild, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

//__________________________________________________________________________________________________
// Mix-in class to any child window.
//
class WindowCtrl : virtual public Window {
public:
	virtual ~WindowCtrl() = 0;
protected:
	bool _drawBorders(WPARAM wp, LPARAM lp);
};