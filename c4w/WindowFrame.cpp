/*!
 * Regular windows, created through CreateWindowEx and using a WNDPROC.
 * Part of C4W - Classes for Win32.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/c4w
 */

#include "WindowFrame.h"
#include "Resources.h"
#include "Str.h"
using namespace c4w;

Frame::~Frame()
{
}

ATOM Frame::Register(const wchar_t *className, int iconId, sys::Cursor cursor, sys::Color bg)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	WNDCLASSEX wc = { 0 };

	wc.cbSize        = sizeof(wc);
	wc.lpfnWndProc   = Frame::_WindowProc;
	wc.hInstance     = hInst;
	wc.lpszClassName = className;
	wc.hbrBackground = reinterpret_cast<HBRUSH>(static_cast<int>(bg) + 1); // http://www.newobjects.com/pages/ndl/alp%5Caf-sysColor.htm
	wc.hCursor       = LoadCursor(nullptr, MAKEINTRESOURCE(cursor));
	wc.style         = CS_DBLCLKS;

	if (iconId) {
		wc.hIcon = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
		wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
	}

	ATOM atom = RegisterClassEx(&wc); // user should register only once, then keep the atom for further calls
	return (!atom && GetLastError() == ERROR_CLASS_ALREADY_EXISTS) ?
		static_cast<ATOM>(GetClassInfoEx(hInst, className, &wc)) : // http://blogs.msdn.com/b/oldnewthing/archive/2004/10/11/240744.aspx
		atom;
}

LRESULT Frame::wndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(this->hWnd(), msg, wp, lp); // default-most message processing for a regular window
}

LRESULT CALLBACK Frame::_WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	Frame *pSelf; // in run-time, will be a pointer to the derived-most class
	if (msg == WM_NCCREATE) {
		pSelf = reinterpret_cast<Frame*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams); // passed on CreateWindowEx()
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf)); // store pointer to object into HWND room
		*static_cast<Window*>(pSelf) = hwnd; // assign hWnd member
	} else {
		pSelf = reinterpret_cast<Frame*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)); // from HWND room, zero if not set yet
	}
	return pSelf ? pSelf->wndProc(msg, wp, lp) : DefWindowProc(hwnd, msg, wp, lp); // works, since wndProc() is virtual
}


FramePopup::~FramePopup()
{
}

LRESULT FramePopup::wndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	static Font hSysFont; // to be shared among all regular container windows

	switch (msg)
	{
	case WM_CREATE:
		if (!hSysFont.hFont()) {
			Font::Info nfof = Font::GetDefaultDialogFontInfo();
			hSysFont.create(nfof);
		}
		hSysFont.applyOnChildren(this->hWnd());
		this->_setWheelHoverBehavior(); // mousewheel working on cursor hovering, not only when control is focused

		SetFocus(GetNextDlgTabItem(this->hWnd(), nullptr, FALSE)); // focus 1st child according to tab order
		this->sendMessage(WM_INITDIALOG, 0, 0); // can be used to process stuff after the WM_CREATE default processing (font & focus)
		break;

	case WM_ACTIVATE:
		if (!HIWORD(wp)) { // it not in minimized state
			if (LOWORD(wp) == WA_INACTIVE) {
				this->_hWndCurFocus = GetFocus(); // save currently focused window
			} else {
				SetFocus(this->_hWndCurFocus); // restore focus back
			}
			return 0;
		}
		break;

	case WindowPopup::SENDORPOSTMSG:
		this->_handleSendOrPostFunction(lp); // for tunelling a callback from another thread
		break;
	}
	return Frame::wndProc(msg, wp, lp); // forward to parent class message handler
}

Window FramePopup::createChild(const wchar_t *className, int id, const wchar_t *caption, DWORD exStyle, DWORD style, POINT pos, SIZE size, LPVOID lp)
{
	return Window(CreateWindowEx(exStyle, className, caption, style, pos.x, pos.y, size.cx, size.cy,
		this->hWnd(), reinterpret_cast<HMENU>(id), this->getInstance(), lp));
}


FrameApp::~FrameApp()
{
}

int FrameApp::run(HINSTANCE hInst, int cmdShow)
{
	InitCommonControls();
	
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_MINIMIZEBOX | _resz | _maximize;
	RECT rc = { 0, 0, _sz.cx, _sz.cy };
	AdjustWindowRect(&rc, dwStyle, _hMenu != nullptr); // compensate different theme window borders
	
	if (!CreateWindowEx(_dropFiles, MAKEINTATOM(_atom), L"", dwStyle, // title can be set with setText()
		rc.left + GetSystemMetrics(SM_CXSCREEN) / 2 - _sz.cx / 2, // center on screen
		rc.top + GetSystemMetrics(SM_CYSCREEN) / 2 - _sz.cy / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, _hMenu, hInst, static_cast<LPVOID>(this)) ) // pass pointer to object, hWnd is set on WM_NCCREATE
	{
		OutputDebugString(str::Sprintf(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError()).c_str());
		return -1;
	}

	ShowWindow(this->hWnd(), cmdShow);
	UpdateWindow(this->hWnd());

	MSG msg = { 0 };
	BOOL ret = 0;
	while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) {
			OutputDebugString(str::Sprintf(L"ERROR: GetMessage failed, error #%d.\n", GetLastError()).c_str());
			return -1;
		}
		if (_hAccelTable && TranslateAccelerator(this->hWnd(), _hAccelTable, &msg)) continue;
		if (IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam); // this can be used as program return value
}

LRESULT FrameApp::wndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return FramePopup::wndProc(msg, wp, lp); // forward to parent class message handler
}


FrameModal::~FrameModal()
{
}

void FrameModal::show(Window *parent)
{
	RECT rcP = parent->getWindowRect();
	DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_VISIBLE | _resz | _maximize;
	RECT rc = { 0, 0, _sz.cx, _sz.cy };
	AdjustWindowRect(&rc, dwStyle, FALSE); // compensate different theme window borders

	if (!CreateWindowEx(WS_EX_DLGMODALFRAME | _dropFiles, MAKEINTATOM(_atom), L"", dwStyle, // title can be set with setText()
		(rcP.right - rcP.left) / 2 + rcP.left - _sz.cx / 2, // center on parent
		(rcP.bottom - rcP.top) / 2 + rcP.top - _sz.cy / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		parent->hWnd(), nullptr, parent->getInstance(),
		static_cast<LPVOID>(this)) ) // pass pointer to object, hWnd is set on WM_NCCREATE
	{
		OutputDebugString(str::Sprintf(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError()).c_str());
		return;
	}

	// Parent is turned back active during WM_CLOSE processing.
	// http://blogs.msdn.com/b/oldnewthing/archive/2005/02/18/376080.aspx
	parent->setEnable(false);

	// A new loop, so caller function is blocked and awaits modal to be closed.
	MSG msg = { 0 };
	while (IsWindow(this->hWnd()) && GetMessage(&msg, nullptr, 0, 0)) {
		if (!(_hAccelTable && TranslateAccelerator(this->hWnd(), _hAccelTable, &msg)) &&
			!IsDialogMessage(this->hWnd(), &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT FrameModal::wndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
		this->getParent().setEnable(true); // re-enable parent window
		DestroyWindow(this->hWnd());
		break;
	}
	return FramePopup::wndProc(msg, wp, lp); // forward to parent class message handler
}


FrameCtrl::~FrameCtrl()
{
}

void FrameCtrl::create(Window *parent, int ctrlId, POINT pos, SIZE sz)
{
	if (!CreateWindowEx(_border | _container, // http://blogs.msdn.com/b/oldnewthing/archive/2004/07/30/201988.aspx
		MAKEINTATOM(_atom), nullptr,
		CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | _tabStop | _scroll,
		pos.x, pos.y, sz.cx, sz.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(ctrlId), parent->getInstance(),
		static_cast<LPVOID>(this)) ) // pass pointer to object; hWnd is set on WM_NCCREATE
	{
		OutputDebugString(str::Sprintf(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError()).c_str());
	}
}

LRESULT FrameCtrl::wndProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_NCPAINT:
		if (this->_drawBorders(wp, lp)) return 0; // themed borders
		break;
	}
	return Frame::wndProc(msg, wp, lp); // forward to parent class message handler
}