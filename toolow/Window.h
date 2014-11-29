//
// HWND wrapper.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#pragma once
#include "String.h"
#include <CommCtrl.h>

//__________________________________________________________________________________________________
// Base HWND wrapper.
//
class Window {
private:
	HWND _hWnd;
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
	Window&   setText(const String& text)                       { return setText(text.str()); }
	wchar_t*  getText(wchar_t *pBuf, int szBuf) const           { ::GetWindowText(_hWnd, pBuf, szBuf); return pBuf; }
	String&   getText(String& buf) const                        { buf.reserve(::GetWindowTextLength(_hWnd)); ::GetWindowText(_hWnd, buf.ptrAt(0), buf.reserved() + 1); return buf; }
	String    getText() const                                   { String ret; getText(ret); return ret; }
	Window&   setPos(HWND hInsertAfter, int x, int y, int cx, int cy, UINT flags) { ::SetWindowPos(_hWnd, hInsertAfter, x, y, cx, cy, flags); return *this; }
	Window&   setFocus()                                        { ::SetFocus(_hWnd); return *this; }
	bool      isFocused() const                                 { return ::GetFocus() == _hWnd; }
	Window&   setVisible(bool visible)                          { ::ShowWindow(_hWnd, visible ? SW_SHOW : SW_HIDE); return *this; }
	bool      isVisible() const                                 { return ::IsWindowVisible(_hWnd) == TRUE; }
	Window&   setEnable(bool doEnable)                          { ::EnableWindow(_hWnd, doEnable); return *this; }
	bool      isEnabled() const                                 { return ::IsWindowEnabled(_hWnd) == TRUE; }
	RECT      getClientRect() const                             { RECT r = { 0 }; ::GetClientRect(_hWnd, &r); return r; }
	RECT      getWindowRect() const                             { RECT r = { 0 }; ::GetWindowRect(_hWnd, &r); return r; }
	Window&   screenToClient(POINT *pPt)                        { ::ScreenToClient(_hWnd, pPt); return *this; }
	Window&   screenToClient(RECT *pRc)                         { screenToClient((POINT*)&pRc->left); return screenToClient((POINT*)&pRc->right); }
	Window&   clientToScreen(POINT *pPt)                        { ::ClientToScreen(_hWnd, pPt); return *this; }
	Window&   clientToScreen(RECT *pRc)                         { clientToScreen((POINT*)&pRc->left); return clientToScreen((POINT*)&pRc->right); }
};

//__________________________________________________________________________________________________
// Mix-in class to any popup window.
// To remove icon from a resizing modal, use EXSTYLE WS_EX_DLGMODALFRAME.
//
class WindowPopup : virtual public Window {
public:
	virtual ~WindowPopup() = 0;
	void sendFunction(function<void()> callback) { _sendOrPostFunction(MOVE(callback), true); }
	void postFunction(function<void()> callback) { _sendOrPostFunction(MOVE(callback), false); }
protected:
	Window getChild(int id) { return Window(::GetDlgItem(hWnd(), id)); }
	bool   isMinimized()    { return ::IsIconic(hWnd()) == TRUE; }
	bool   isMaximized()    { return ::IsZoomed(hWnd()) == TRUE; }
	int    messageBox(const wchar_t *caption, const wchar_t *body, UINT uType=0);
	int    messageBox(const wchar_t *caption, const String& body, UINT uType=0) { return messageBox(caption, body.str(), uType); }
	bool   getFileOpen(const wchar_t *filter, String& buf);
	bool   getFileOpen(const wchar_t *filter, Array<String>& arrBuf);
	bool   getFileSave(const wchar_t *filter, String& buf, const wchar_t *defFile=nullptr);
	bool   getFolderChoose(String& buf);
	void   setXButton(bool enable);
	Array<String> getDroppedFiles(HDROP hDrop);
	void   _setWheelHoverBehavior();
	void   _handleSendOrPostFunction(LPARAM lp);
private:
	void   _sendOrPostFunction(function<void()> callback, bool isSend);
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