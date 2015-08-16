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
#include "res.h"

namespace wolf {
namespace wnd {

/// Base HWND wrapper.
class Wnd {
private:
	HWND _hWnd;
public:
	Wnd()                 : _hWnd(nullptr) { }
	Wnd(HWND hwnd)        : _hWnd(hwnd) { }
	Wnd(const Wnd& other) : _hWnd(other._hWnd) { }
	virtual ~Wnd()        { }

	HWND         hWnd() const                { return _hWnd; }
	Wnd&         operator=(HWND hwnd)        { _hWnd = hwnd; return *this; }
	Wnd&         operator=(const Wnd& other) { _hWnd = other._hWnd; return *this; }
	Wnd&         create(const wchar_t *className, int id, const wchar_t *caption, Wnd *parent, DWORD exStyle, DWORD style, POINT pos, SIZE size, LPVOID lp=nullptr);
	Wnd          getChild(int id) const      { return Wnd(::GetDlgItem(hWnd(), id)); }
	Wnd          getParent() const           { return ::GetParent(_hWnd); }
	HINSTANCE    getInstance() const         { return reinterpret_cast<HINSTANCE>(::GetWindowLongPtr(_hWnd, GWLP_HINSTANCE)); }
	int          getId() const               { return ::GetDlgCtrlID(_hWnd); }
	LRESULT      postMessage(UINT msg, WPARAM wp, LPARAM lp) const       { return ::PostMessage(_hWnd, msg, wp, lp); }
	LRESULT      sendMessage(UINT msg, WPARAM wp, LPARAM lp) const       { return ::SendMessage(_hWnd, msg, wp, lp); }
	LRESULT      sendCommand(WORD cmd, WORD hiwordWp, LPARAM lp) const   { return sendMessage(WM_COMMAND, MAKEWPARAM(cmd, hiwordWp), lp); }
	LRESULT      sendNotify(UINT_PTR idFrom, UINT code, NMHDR *nm) const { return sendMessage(WM_NOTIFY, idFrom, reinterpret_cast<LPARAM>(nm)); }
	Wnd&         setText(const wchar_t *text)            { ::SetWindowText(_hWnd, text); return *this; }
	Wnd&         setText(const std::wstring& text)       { return setText(text.c_str()); }
	wchar_t*     getText(wchar_t *pBuf, int szBuf) const { ::GetWindowText(_hWnd, pBuf, szBuf); return pBuf; }
	std::wstring getText() const;
	Wnd&         setPos(HWND hInsertAfter, POINT pos, SIZE sz, UINT flags) { ::SetWindowPos(_hWnd, hInsertAfter, pos.x, pos.y, sz.cx, sz.cy, flags); return *this; }
	Wnd&         setFocus()                 { ::SetFocus(_hWnd); return *this; }
	bool         isFocused() const          { return ::GetFocus() == _hWnd; }
	Wnd&         setVisible(bool visible)   { ::ShowWindow(_hWnd, visible ? SW_SHOW : SW_HIDE); return *this; }
	bool         isVisible() const          { return ::IsWindowVisible(_hWnd) == TRUE; }
	Wnd&         setEnable(bool doEnable)   { ::EnableWindow(_hWnd, doEnable); return *this; }
	bool         isEnabled() const          { return ::IsWindowEnabled(_hWnd) == TRUE; }
	RECT         getClientRect() const      { RECT r = { 0 }; ::GetClientRect(_hWnd, &r); return r; }
	RECT         getWindowRect() const      { RECT r = { 0 }; ::GetWindowRect(_hWnd, &r); return r; }
	Wnd&         screenToClient(POINT *pPt) { ::ScreenToClient(_hWnd, pPt); return *this; }
	Wnd&         screenToClient(RECT *pRc)  { screenToClient(reinterpret_cast<POINT*>(&pRc->left)); return screenToClient(reinterpret_cast<POINT*>(&pRc->right)); }
	Wnd&         clientToScreen(POINT *pPt) { ::ClientToScreen(_hWnd, pPt); return *this; }
	Wnd&         clientToScreen(RECT *pRc)  { clientToScreen(reinterpret_cast<POINT*>(&pRc->left)); return clientToScreen(reinterpret_cast<POINT*>(&pRc->right)); }
};


/// Mix-in class to any top-level window.
// To remove icon from a resizing modal, use EXSTYLE WS_EX_DLGMODALFRAME.
class TopLevel : virtual public Wnd {
public:
	virtual ~TopLevel() = 0;
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
	void inOrigThread(std::function<void()> callback, bool synchronous=true);
protected:
	void _setWheelHoverBehavior();
	void _handleOrigThread(LPARAM lp);
	enum { _WM_ORIGTHREAD=WM_APP-1 };
};


/// Mix-in class to any child control window.
class Child : virtual public Wnd {
public:
	virtual ~Child() = 0;
protected:
	bool _drawThemeBorders(WPARAM wp, LPARAM lp);
};

}//namespace wnd
}//namespace wolf