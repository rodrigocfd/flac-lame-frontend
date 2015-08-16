/*!
 * @file
 * @brief Regular windows, created through CreateWindowEx and using a WNDPROC.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "wnd_window.h"
#include "str.h"
using namespace wolf;
using namespace wolf::res;
using namespace wolf::wnd;

Window::~Window()
{
}

void Window::invalidateRect(bool bgErase)
{
	InvalidateRect(this->hWnd(), 0, static_cast<BOOL>(bgErase));
}

LRESULT Window::defWindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(this->hWnd(), msg, wp, lp);
}

ATOM Window::Register(const wchar_t *className, int iconId, sys::Cursor cursor, sys::Color bg)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	WNDCLASSEX wc = { 0 };

	wc.cbSize        = sizeof(wc);
	wc.lpfnWndProc   = Window::_WindowProc;
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

LRESULT CALLBACK Window::_WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	Window *pSelf = nullptr; // in run-time, will be a pointer to the derived-most class
	if (msg == WM_NCCREATE) {
		pSelf = reinterpret_cast<Window*>((reinterpret_cast<CREATESTRUCT*>(lp))->lpCreateParams); // passed on CreateWindowEx()
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf)); // store pointer to object into HWND room
		*static_cast<Wnd*>(pSelf) = hwnd; // assign hWnd member
	} else {
		pSelf = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)); // from HWND room, zero if not set yet
	}

	LRESULT ret = 0;
	if (pSelf) {
		ret = pSelf->_processMessage(msg, wp, lp);
		if (msg == WM_CREATE) pSelf->_onCreate(); // after user's
	} else {
		ret = DefWindowProc(hwnd, msg, wp, lp);
	}
	return ret;
}

void Window::_internalEvents()
{
	EventWindow::_internalEvents();
}


WindowTopLevel::~WindowTopLevel()
{
}

void WindowTopLevel::_onCreate()
{
	static Font hSysFont; // to be shared among all regular frame windows

	if (!hSysFont.hFont()) {
		Font::Info nfof = Font::GetDefaultDialogFontInfo();
		hSysFont.create(nfof);
	}
	hSysFont.applyOnChildren(this->hWnd());
	
	this->_setWheelHoverBehavior(); // mousewheel working on cursor hovering, not only when control is focused
	SetFocus(GetNextDlgTabItem(this->hWnd(), nullptr, FALSE)); // focus 1st child according to tab order

	this->sendMessage(WM_INITDIALOG, 0, 0); // can be used to process stuff after the WM_CREATE default processing (font & focus)
}

void WindowTopLevel::_internalEvents()
{
	Window::_internalEvents();

	this->onMessage(WM_ACTIVATE, [&](WPARAM wp, LPARAM lp)->LRESULT {
		if (!HIWORD(wp)) { // it not in minimized state
			if (LOWORD(wp) == WA_INACTIVE) {
				this->_hWndCurFocus = GetFocus(); // save currently focused window
			} else {
				SetFocus(this->_hWndCurFocus); // restore focus back
			}
			return 0;
		}
		return this->defWindowProc(WM_ACTIVATE, wp, lp);
	});

	this->onMessage(WindowTopLevel::_WM_ORIGTHREAD, [&](WPARAM wp, LPARAM lp)->LRESULT {
		this->_handleOrigThread(lp); // for tunelling a callback from another thread
		return 0;
	});
}


WindowMain::~WindowMain()
{
}

int WindowMain::run(HINSTANCE hInst, int cmdShow)
{
	InitCommonControls();
	this->_internalEvents();
	this->events(); // attach all user event messages
	
	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_MINIMIZEBOX | _resz | _maximize;
	RECT rc = { 0, 0, _sz.cx, _sz.cy };
	AdjustWindowRect(&rc, dwStyle, _hMenu != nullptr); // compensate different theme window borders
	
	if (!CreateWindowEx(_dropFiles, MAKEINTATOM(_atom), L"", dwStyle, // title can be set with setText()
		rc.left + GetSystemMetrics(SM_CXSCREEN) / 2 - _sz.cx / 2, // center on screen
		rc.top + GetSystemMetrics(SM_CYSCREEN) / 2 - _sz.cy / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, _hMenu, hInst, static_cast<LPVOID>(this)) ) // pass pointer to object, hWnd is set on WM_NCCREATE
	{
		str::Dbg(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError());
		return -1;
	}

	if (mainMenu.hMenu()) SetMenu(this->hWnd(), mainMenu.hMenu());
	ShowWindow(this->hWnd(), cmdShow);
	UpdateWindow(this->hWnd());

	MSG msg = { 0 };
	BOOL ret = 0;
	while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) {
			str::Dbg(L"ERROR: GetMessage failed, error #%d.\n", GetLastError());
			return -1;
		}
		if (accelTable.hAccel() && TranslateAccelerator(this->hWnd(), accelTable.hAccel(), &msg)) continue;
		if (IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam); // this can be used as program return value
}

void WindowMain::_onCreate()
{
	WindowTopLevel::_onCreate();
}

void WindowMain::_internalEvents()
{
	WindowTopLevel::_internalEvents();

	this->onMessage(WM_DESTROY, [&](WPARAM wp, LPARAM lp)->LRESULT {
		PostQuitMessage(0);
		return 0;
	});
}


WindowModal::~WindowModal()
{
}

void WindowModal::show(Wnd *owner)
{
	this->_internalEvents();
	this->events(); // attach all user event messages

	RECT rcP = owner->getWindowRect();
	DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_VISIBLE | _resz | _maximize;
	RECT rc = { 0, 0, _sz.cx, _sz.cy };
	AdjustWindowRect(&rc, dwStyle, FALSE); // compensate different theme window borders

	if (!CreateWindowEx(WS_EX_DLGMODALFRAME | _dropFiles, MAKEINTATOM(_atom), L"", dwStyle, // title can be set with setText()
		(rcP.right - rcP.left) / 2 + rcP.left - _sz.cx / 2, // center on parent
		(rcP.bottom - rcP.top) / 2 + rcP.top - _sz.cy / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		owner->hWnd(), nullptr, owner->getInstance(),
		static_cast<LPVOID>(this)) ) // pass pointer to object, hWnd is set on WM_NCCREATE
	{
		str::Dbg(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError());
		return;
	}

	if (mainMenu.hMenu()) SetMenu(this->hWnd(), mainMenu.hMenu());

	// Parent is turned back active during WM_CLOSE processing.
	// http://blogs.msdn.com/b/oldnewthing/archive/2005/02/18/376080.aspx
	owner->setEnable(false);

	// A new loop, so caller function is blocked and awaits modal to be closed.
	MSG msg = { 0 };
	BOOL ret = 0;
	while (IsWindow(this->hWnd()) && (ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) {
			str::Dbg(L"ERROR: GetMessage failed, error #%d.\n", GetLastError());
			return;
		}
		if (accelTable.hAccel() && TranslateAccelerator(this->hWnd(), accelTable.hAccel(), &msg)) continue;
		if (IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void WindowModal::_onCreate()
{
	WindowTopLevel::_onCreate();
}

void WindowModal::_internalEvents()
{
	WindowTopLevel::_internalEvents();

	this->onMessage(WM_CLOSE, [&](WPARAM wp, LPARAM lp)->LRESULT {
		this->getParent().setEnable(true); // re-enable parent window
		DestroyWindow(this->hWnd());
		return 0;
	});
}


WindowChild::~WindowChild()
{
}

void WindowChild::create(Wnd *parent, int ctrlId, POINT pos, SIZE sz)
{
	this->_internalEvents();
	this->events(); // attach all user event messages

	if (!CreateWindowEx(_border | _container, // http://blogs.msdn.com/b/oldnewthing/archive/2004/07/30/201988.aspx
		MAKEINTATOM(_atom), nullptr,
		CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | _tabStop | _scroll,
		pos.x, pos.y, sz.cx, sz.cy,
		parent->hWnd(), reinterpret_cast<HMENU>(ctrlId), parent->getInstance(),
		static_cast<LPVOID>(this)) ) // pass pointer to object; hWnd is set on WM_NCCREATE
	{
		str::Dbg(L"ERROR: CreateWindowEx failed, error #%d.\n", GetLastError());
	}
}

void WindowChild::getScrollInfo(int fnBar, SCROLLINFO& si)
{
	GetScrollInfo(this->hWnd(), fnBar, &si);
}

int WindowChild::setScrollInfo(int fnBar, const SCROLLINFO& si, bool redraw)
{
	return SetScrollInfo(this->hWnd(), fnBar, &si, static_cast<BOOL>(redraw));
}

void WindowChild::_onCreate()
{
}

void WindowChild::_internalEvents()
{
	Window::_internalEvents();

	this->onMessage(WM_NCPAINT, [&](WPARAM wp, LPARAM lp)->LRESULT {
		return this->_drawThemeBorders(wp, lp) ?
			0 : this->defWindowProc(WM_NCPAINT, wp, lp);
	});
}