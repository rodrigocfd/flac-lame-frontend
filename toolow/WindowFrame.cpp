//
// Realm of the regular windows, those who take a WNDPROC as a procedure and are created through CreateWindowEx().
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "WindowFrame.h"
#include "Font.h"

Frame::~Frame()
{
}

ATOM Frame::Register(const wchar_t *className, System::Cursor cursor, int iconId, System::Color bg)
{
	HINSTANCE hInst = GetModuleHandle(nullptr);
	WNDCLASSEX wc = { 0 };

	wc.cbSize        = sizeof(wc);
	wc.lpfnWndProc   = Frame::_WindowProc;
	wc.hInstance     = hInst;
	wc.lpszClassName = className;
	wc.hbrBackground = (HBRUSH)((int)bg + 1); // http://www.newobjects.com/pages/ndl/alp%5Caf-sysColor.htm
	wc.hCursor       = LoadCursor(nullptr, MAKEINTRESOURCE(cursor));
	wc.style         = CS_DBLCLKS;

	if (iconId) {
		wc.hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
		wc.hIconSm = (HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}

	return RegisterClassEx(&wc);
}

LRESULT Frame::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	return DefWindowProc(this->hWnd(), msg, wp, lp); // default-most message processing for a regular window
}

LRESULT CALLBACK Frame::_WindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	Frame *pSelf; // in run-time, will be a pointer to the derived-most class
	if (msg == WM_NCCREATE) {
		pSelf = (Frame*)((CREATESTRUCT*)lp)->lpCreateParams; // passed on CreateWindowEx()
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pSelf); // store pointer to object into HWND room
		*(Window*)pSelf = hwnd; // assign hWnd member
	}
	else pSelf = (Frame*)GetWindowLongPtr(hwnd, GWLP_USERDATA); // from HWND room, zero if not set yet
	return pSelf ? pSelf->msgHandler(msg, wp, lp) : DefWindowProc(hwnd, msg, wp, lp); // works, since msgHandler() is virtual
}


FramePopup::~FramePopup()
{
}

LRESULT FramePopup::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
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
			if (LOWORD(wp) == WA_INACTIVE)
				this->_hWndCurFocus = GetFocus(); // save currently focused window
			else
				SetFocus(this->_hWndCurFocus); // restore focus back
			return 0;
		}
		break;

	case WM_APP-1:
		this->_handleSendOrPostFunction(lp); // for tunelling a callback from another thread
		break;
	}
	return Frame::msgHandler(msg, wp, lp); // forward to parent class message handler
}

Window FramePopup::createButton(const wchar_t *caption, int id, int x, int y, int cx, bool def)
{
	return CreateWindowEx(0, L"BUTTON", caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | (def ? BS_DEFPUSHBUTTON : 0), x, y, cx, 23,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createLabel(const wchar_t *caption, int x, int y, int cx, int id)
{
	return CreateWindowEx(0, L"STATIC", caption,
		WS_CHILD | WS_VISIBLE, x, y, cx, 17,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createCheck(const wchar_t *caption, int id, int x, int y, int cx)
{
	return CreateWindowEx(0, L"BUTTON", caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, cx, 21,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createEdit(int id, int x, int y, int cx, UINT extraStyles)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | extraStyles, x, y, cx, 21,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}

Window FramePopup::createCombo(int id, int x, int y, int cx)
{
	return CreateWindowEx(0, L"COMBOBOX", nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | CBS_SORT, x, y, cx, 0,
		this->hWnd(), (HMENU)id, this->getInstance(), 0);
}


FrameApp::~FrameApp()
{
}

int FrameApp::run(HINSTANCE hInst, int cmdShow, ATOM atom, const wchar_t *caption, int cxClient, int cyClient,
	FramePopup::Style::Maximize maximizable, FramePopup::Style::Resize resizable,
	FramePopup::Style::Drop droppable, HMENU hMenu, HACCEL hAccel)
{
	InitCommonControls();

	DWORD dwStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_MINIMIZEBOX | maximizable | resizable;
	RECT rc = { 0, 0, cxClient, cyClient };
	AdjustWindowRect(&rc, dwStyle, hMenu != 0); // compensate different theme window borders

	if (!CreateWindowEx(droppable, // hWnd is set on WM_NCCREATE
		(LPCWSTR)MAKELONG(atom, 0), caption, dwStyle,
		rc.left + GetSystemMetrics(SM_CXSCREEN) / 2 - cxClient / 2, // center on screen
		rc.top + GetSystemMetrics(SM_CYSCREEN) / 2 - cyClient / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		nullptr, hMenu, hInst, (LPVOID)this) ) // pass pointer to object
	{
		return -1; // window creation failed
	}

	ShowWindow(this->hWnd(), cmdShow);
	UpdateWindow(this->hWnd());

	MSG msg = { 0 };
	BOOL ret = 0;
	while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) return -1; // failure
		if (hAccel && TranslateAccelerator(this->hWnd(), hAccel, &msg)) continue;
		if (IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam; // this can be used as program return value
}

LRESULT FrameApp::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return FramePopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}


FrameModal::~FrameModal()
{
}

int FrameModal::show(Window *parent, ATOM atom, const wchar_t *caption, int cxClient, int cyClient,
	FramePopup::Style::Resize resizable, HACCEL hAccel)
{
	RECT rcP = parent->getWindowRect();
	DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_VISIBLE | resizable;
	RECT rc = { 0, 0, cxClient, cyClient };
	AdjustWindowRect(&rc, dwStyle, FALSE); // compensate different theme window borders

	CreateWindowEx(WS_EX_DLGMODALFRAME, // hWnd is set on WM_NCCREATE
		(LPCWSTR)MAKELONG(atom, 0), caption, dwStyle,
		(rcP.right - rcP.left) / 2 + rcP.left - cxClient / 2, // center on parent
		(rcP.bottom - rcP.top) / 2 + rcP.top - cyClient / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		parent->hWnd(), nullptr, parent->getInstance(),
		(LPVOID)this); // pass pointer to object

	// Parent is turned back active durint WM_CLOSE processing.
	// http://blogs.msdn.com/b/oldnewthing/archive/2005/02/18/376080.aspx
	parent->setEnable(false);

	// A new loop, so caller function is blocked and awaits modal to be closed.
	MSG msg = { 0 };
	while (IsWindow(this->hWnd()) && GetMessage(&msg, nullptr, 0, 0)) {
		if (!(hAccel && TranslateAccelerator(this->hWnd(), hAccel, &msg)) && !IsDialogMessage(this->hWnd(), &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

LRESULT FrameModal::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
		this->getParent().setEnable(true); // re-enable parent window
		DestroyWindow(this->hWnd());
		break;
	}
	return FramePopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}


FrameCtrl::~FrameCtrl()
{
}

void FrameCtrl::create(int id, Window *parent, ATOM atom, int x, int y, int cx, int cy,
	FrameCtrl::Style::Border border, FrameCtrl::Style::Container container,
	FrameCtrl::Style::Scroll scroll, FrameCtrl::Style::TabStop tabStop)
{
	CreateWindowEx(border | container, // http://blogs.msdn.com/b/oldnewthing/archive/2004/07/30/201988.aspx
		(LPCWSTR)MAKELONG(atom, 0), nullptr,
		CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | scroll | tabStop,
		x, y, cx, cy,
		parent->hWnd(), (HMENU)id, parent->getInstance(),
		(LPVOID)this); // pass pointer to object; hWnd is set on WM_NCCREATE
}

LRESULT FrameCtrl::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_NCPAINT:
		if (this->_drawBorders(wp, lp)) // themed borders
			return 0;
		break;
	}
	return Frame::msgHandler(msg, wp, lp); // forward to parent class message handler
}