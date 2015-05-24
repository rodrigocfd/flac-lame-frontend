/*!
 * HWND wrapper.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>

namespace c4w {

// Base HWND wrapper.
class Window {
private:
	HWND _hWnd;
public:
	Window()                    : _hWnd(nullptr) { }
	Window(HWND hwnd)           : _hWnd(hwnd) { }
	Window(const Window& other) : _hWnd(other._hWnd) { }

	HWND         hWnd() const                                      { return _hWnd; }
	Window&      operator=(HWND hwnd)                              { _hWnd = hwnd; return *this; }
	Window&      operator=(const Window& other)                    { _hWnd = other._hWnd; return *this; }
	Window       getParent() const                                 { return ::GetParent(_hWnd); }
	HINSTANCE    getInstance() const                               { return reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)); }
	int          getId() const                                     { return ::GetDlgCtrlID(_hWnd); }
	LRESULT      sendMessage(UINT msg, WPARAM wp, LPARAM lp) const { return ::SendMessage(_hWnd, msg, wp, lp); }
	LRESULT      postMessage(UINT msg, WPARAM wp, LPARAM lp) const { return ::PostMessage(_hWnd, msg, wp, lp); }
	Window&      setText(const wchar_t *text)                      { ::SetWindowText(_hWnd, text); return *this; }
	Window&      setText(const std::wstring& text)                 { return setText(text.c_str()); }
	wchar_t*     getText(wchar_t *pBuf, int szBuf) const           { ::GetWindowText(_hWnd, pBuf, szBuf); return pBuf; }
	std::wstring getText() const;
	Window&      setPos(HWND hInsertAfter, POINT pos, SIZE sz, UINT flags) { ::SetWindowPos(_hWnd, hInsertAfter, pos.x, pos.y, sz.cx, sz.cy, flags); return *this; }
	Window&      setFocus()                                        { ::SetFocus(_hWnd); return *this; }
	bool         isFocused() const                                 { return ::GetFocus() == _hWnd; }
	Window&      setVisible(bool visible)                          { ::ShowWindow(_hWnd, visible ? SW_SHOW : SW_HIDE); return *this; }
	bool         isVisible() const                                 { return ::IsWindowVisible(_hWnd) == TRUE; }
	Window&      setEnable(bool doEnable)                          { ::EnableWindow(_hWnd, doEnable); return *this; }
	bool         isEnabled() const                                 { return ::IsWindowEnabled(_hWnd) == TRUE; }
	RECT         getClientRect() const                             { RECT r = { 0 }; ::GetClientRect(_hWnd, &r); return r; }
	RECT         getWindowRect() const                             { RECT r = { 0 }; ::GetWindowRect(_hWnd, &r); return r; }
	Window&      screenToClient(POINT *pPt)                        { ::ScreenToClient(_hWnd, pPt); return *this; }
	Window&      screenToClient(RECT *pRc)                         { screenToClient(reinterpret_cast<POINT*>(&pRc->left)); return screenToClient(reinterpret_cast<POINT*>(&pRc->right)); }
	Window&      clientToScreen(POINT *pPt)                        { ::ClientToScreen(_hWnd, pPt); return *this; }
	Window&      clientToScreen(RECT *pRc)                         { clientToScreen(reinterpret_cast<POINT*>(&pRc->left)); return clientToScreen(reinterpret_cast<POINT*>(&pRc->right)); }
};


// Mix-in class to any popup window.
// To remove icon from a resizing modal, use EXSTYLE WS_EX_DLGMODALFRAME.
class WindowPopup : virtual public Window {
public:
	virtual ~WindowPopup() = 0;
protected:
	Window getChild(int id) { return Window(::GetDlgItem(hWnd(), id)); }
	bool   isMinimized()    { return ::IsIconic(hWnd()) == TRUE; }
	bool   isMaximized()    { return ::IsZoomed(hWnd()) == TRUE; }
	int    messageBox(const wchar_t *caption, const wchar_t *body, UINT uType=0);
	int    messageBox(const wchar_t *caption, const std::wstring& body, UINT uType=0)      { return messageBox(caption, body.c_str(), uType); }
	int    messageBox(const std::wstring& caption, const std::wstring& body, UINT uType=0) { return messageBox(caption.c_str(), body.c_str(), uType); }
	bool   getFileOpen(const wchar_t *filter, std::wstring& buf);
	bool   getFileOpen(const wchar_t *filter, std::vector<std::wstring>& arrBuf);
	bool   getFileSave(const wchar_t *filter, std::wstring& buf, const wchar_t *defFile=nullptr);
	bool   getFolderChoose(std::wstring& buf);
	void   setXButton(bool enable);
	std::vector<std::wstring> getDroppedFiles(HDROP hDrop);
	void   _setWheelHoverBehavior();
	void   _handleSendOrPostFunction(LPARAM lp);
	void   sendFunction(std::function<void()> callback) { _sendOrPostFunction(std::move(callback), true); }
	void   postFunction(std::function<void()> callback) { _sendOrPostFunction(std::move(callback), false); }
	enum { SENDORPOSTMSG = WM_APP-1 };
private:
	void   _sendOrPostFunction(std::function<void()> callback, bool isSend);
};


// Mix-in class to any child control window.
class WindowCtrl : virtual public Window {
public:
	virtual ~WindowCtrl() = 0;
protected:
	bool _drawBorders(WPARAM wp, LPARAM lp);
};

}//namespace c4w