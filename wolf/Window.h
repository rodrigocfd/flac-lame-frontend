/*!
 * @file
 * @brief HWND wrapper.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Windows.h>
#include <CommCtrl.h>
#include "Res.h"

namespace wolf {

/// Base HWND wrapper.
class Window {
private:
	HWND _hWnd;
public:
	Window()                    : _hWnd(nullptr) { }
	Window(HWND hwnd)           : _hWnd(hwnd) { }
	Window(const Window& other) : _hWnd(other._hWnd) { }
	virtual ~Window()           { }

	HWND         hWnd() const                   { return _hWnd; }
	Window&      operator=(HWND hwnd)           { _hWnd = hwnd; return *this; }
	Window&      operator=(const Window& other) { _hWnd = other._hWnd; return *this; }
	Window       getChild(int id) const         { return Window(::GetDlgItem(hWnd(), id)); }
	Window       getParent() const              { return ::GetParent(_hWnd); }
	HINSTANCE    getInstance() const            { return reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)); }
	int          getId() const                  { return ::GetDlgCtrlID(_hWnd); }
	LRESULT      postMessage(UINT msg, WPARAM wp, LPARAM lp) const       { return ::PostMessage(_hWnd, msg, wp, lp); }
	LRESULT      sendMessage(UINT msg, WPARAM wp, LPARAM lp) const       { return ::SendMessage(_hWnd, msg, wp, lp); }
	LRESULT      sendCommand(WORD cmd, WORD hiwordWp, LPARAM lp) const   { return sendMessage(WM_COMMAND, MAKEWPARAM(cmd, hiwordWp), lp); }
	LRESULT      sendNotify(UINT_PTR idFrom, UINT code, NMHDR *nm) const { return sendMessage(WM_NOTIFY, idFrom, reinterpret_cast<LPARAM>(nm)); }
	Window&      setText(const wchar_t *text)            { ::SetWindowText(_hWnd, text); return *this; }
	Window&      setText(const std::wstring& text)       { return setText(text.c_str()); }
	wchar_t*     getText(wchar_t *pBuf, int szBuf) const { ::GetWindowText(_hWnd, pBuf, szBuf); return pBuf; }
	std::wstring getText() const;
	Window&      setPos(HWND hInsertAfter, POINT pos, SIZE sz, UINT flags) { ::SetWindowPos(_hWnd, hInsertAfter, pos.x, pos.y, sz.cx, sz.cy, flags); return *this; }
	Window&      setFocus()                 { ::SetFocus(_hWnd); return *this; }
	bool         isFocused() const          { return ::GetFocus() == _hWnd; }
	Window&      setVisible(bool visible)   { ::ShowWindow(_hWnd, visible ? SW_SHOW : SW_HIDE); return *this; }
	bool         isVisible() const          { return ::IsWindowVisible(_hWnd) == TRUE; }
	Window&      setEnable(bool doEnable)   { ::EnableWindow(_hWnd, doEnable); return *this; }
	bool         isEnabled() const          { return ::IsWindowEnabled(_hWnd) == TRUE; }
	RECT         getClientRect() const      { RECT r = { 0 }; ::GetClientRect(_hWnd, &r); return r; }
	RECT         getWindowRect() const      { RECT r = { 0 }; ::GetWindowRect(_hWnd, &r); return r; }
	Window&      screenToClient(POINT *pPt) { ::ScreenToClient(_hWnd, pPt); return *this; }
	Window&      screenToClient(RECT *pRc)  { screenToClient(reinterpret_cast<POINT*>(&pRc->left)); return screenToClient(reinterpret_cast<POINT*>(&pRc->right)); }
	Window&      clientToScreen(POINT *pPt) { ::ClientToScreen(_hWnd, pPt); return *this; }
	Window&      clientToScreen(RECT *pRc)  { clientToScreen(reinterpret_cast<POINT*>(&pRc->left)); return clientToScreen(reinterpret_cast<POINT*>(&pRc->right)); }
};


/// Mix-in class to any top-level window.
// To remove icon from a resizing modal, use EXSTYLE WS_EX_DLGMODALFRAME.
class WindowTopLevel : virtual public Window {
public:
	virtual ~WindowTopLevel() = 0;
	bool isMinimized() const { return ::IsIconic(hWnd()) == TRUE; }
	bool isMaximized() const { return ::IsZoomed(hWnd()) == TRUE; }
protected:
	res::MenuMain mainMenu;
	res::AccelTable accelTable;
	int  messageBox(const std::wstring& caption, const std::wstring& body, UINT uType=0);
	bool getFileOpen(const wchar_t *filter, std::wstring& buf);
	bool getFileOpen(const wchar_t *filter, std::vector<std::wstring>& arrBuf);
	bool getFileSave(const wchar_t *filter, std::wstring& buf, const wchar_t *defFile=nullptr);
	bool getFolderChoose(std::wstring& buf);
	void setXButton(bool enable);
	std::vector<std::wstring> getDroppedFiles(HDROP hDrop);
	void origThreadSync(std::function<void()> callback);
	void origThreadAsync(std::function<void()> callback);
protected:
	void _setWheelHoverBehavior();
	void _handleOrigThread(LPARAM lp);
	enum { _WM_ORIGTHREAD=WM_APP-1 };
};


/// Mix-in class to any child control window.
class WindowChild : virtual public Window {
public:
	virtual ~WindowChild() = 0;
protected:
	bool _drawThemeBorders(WPARAM wp, LPARAM lp);
};

}//namespace wolf