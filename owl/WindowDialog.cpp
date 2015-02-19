/*!
 * Dialog windows, created from a dialog resource and using DLGPROC.
 * Part of OWL - Object Win32 Library.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/owl
 */

#include "WindowDialog.h"
#include "StrUtil.h"
#include "System.h"
using namespace owl;

Dialog::~Dialog()
{
}

INT_PTR Dialog::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	return FALSE; // default-most message processing for a dialog
}

INT_PTR CALLBACK Dialog::_DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	Dialog *pSelf; // in run-time, will be a pointer to the derived-most class
	if (msg == WM_INITDIALOG) {
		pSelf = reinterpret_cast<Dialog*>(lp); // passed on CreateDialogParam() or DialogBoxParam()
		SetWindowLongPtr(hdlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf)); // store pointer to object into HWND room
		*static_cast<Window*>(pSelf) = hdlg; // assign hWnd member
	} else {
		pSelf = reinterpret_cast<Dialog*>(GetWindowLongPtr(hdlg, GWLP_USERDATA)); // from HWND room, zero if not set yet
	}
	return pSelf ? pSelf->dlgProc(msg, wp, lp) : FALSE; // works, since dlgProc() is virtual
}


DialogPopup::~DialogPopup()
{
}

INT_PTR DialogPopup::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	static Font hSysFont; // to be shared among all regular dialog windows

	switch (msg)
	{
	case WM_INITDIALOG:
		if (!hSysFont.hFont()) {
			Font::Info nfof = Font::GetDefaultDialogFontInfo();
			hSysFont.create(nfof);
		}
		hSysFont.applyOnChildren(this->hWnd());
		this->_setWheelHoverBehavior(); // mousewheel working on cursor hovering, not only when control is focused
		break;

	case WindowPopup::SENDORPOSTMSG:
		this->_handleSendOrPostFunction(lp); // for tunelling a callback from another thread
		break;
	}
	return Dialog::dlgProc(msg, wp, lp); // forward to parent class message handler
}

BOOL DialogPopup::endDialog(INT_PTR nResult)
{
	return EndDialog(this->hWnd(), nResult); // should always be called on the same window thread
}


DialogApp::~DialogApp()
{
}

int DialogApp::run(HINSTANCE hInst, int cmdShow)
{
	InitCommonControls();

	// The HWND will be assigned during WM_INITDIALOG message, and when the
	// function below returns, it will be already set and ready to be used.
	if (!CreateDialogParam(hInst, MAKEINTRESOURCE(_dialogId), nullptr,
		Dialog::_DialogProc, reinterpret_cast<LPARAM>(this)) ) // pass pointer to object
	{
		OutputDebugString(Sprintf(L"ERROR: CreateDialogParam failed, error #%d.\n", GetLastError()).c_str());
		return -1; // window creation failed
	}

	if (_iconId) {
		this->sendMessage(WM_SETICON, ICON_SMALL,
			reinterpret_cast<LPARAM>(static_cast<HICON>( LoadImage(hInst,
				MAKEINTRESOURCE(_iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR) )));
		this->sendMessage(WM_SETICON, ICON_BIG,
			reinterpret_cast<LPARAM>(static_cast<HICON>( LoadImage(hInst,
				MAKEINTRESOURCE(_iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR) )));
	}

	if (_menuId) { // can also be set on resource editor
		SetMenu(this->hWnd(), LoadMenu(hInst, MAKEINTRESOURCE(_menuId)));
	}

	HACCEL hAccel = nullptr; // accelerators table
	if (_accelTableId) {
		hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(_accelTableId));
	}

	ShowWindow(this->hWnd(), cmdShow);

	MSG msg = { 0 };
	BOOL ret = 0;
	while ((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if (ret == -1) {
			OutputDebugString(Sprintf(L"ERROR: GetMessage failed, error #%d.\n", GetLastError()).c_str());
			return -1;
		}
		if (hAccel && TranslateAccelerator(this->hWnd(), hAccel, &msg)) continue;
		if (IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return static_cast<int>(msg.wParam); // this can be used as program return value
}

INT_PTR DialogApp::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(this->hWnd());
		return TRUE;
	case WM_NCDESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return DialogPopup::dlgProc(msg, wp, lp); // forward to parent class message handler
}


DialogModal::~DialogModal()
{
}

int DialogModal::show(Window *parent)
{
	return static_cast<int>(DialogBoxParam(parent->getInstance(),
		MAKEINTRESOURCE(_dialogId), parent->hWnd(), Dialog::_DialogProc,
		reinterpret_cast<LPARAM>(this))); // pass pointer to class instance
}

INT_PTR DialogModal::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			RECT rcPop = this->getWindowRect();
			RECT rcParent = this->getParent().getWindowRect(); // all relative to screen
			this->setPos(nullptr,
				{ rcParent.left + (rcParent.right - rcParent.left)/2 - (rcPop.right - rcPop.left)/2,
					rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rcPop.bottom - rcPop.top)/2 },
				{ 0, 0 } , SWP_NOZORDER | SWP_NOSIZE); // center dialog on parent
		}
		break;
	case WM_CLOSE:
		this->endDialog(IDOK);
		return TRUE;
	}
	return DialogPopup::dlgProc(msg, wp, lp); // forward to parent class message handler
}


DialogCtrl::~DialogCtrl()
{
}

void DialogCtrl::create(Window *parent, POINT pos, SIZE sz)
{
	// Dialog styles to be set on VS resource editor:
	// - Border: none
	// - Control: true
	// - Style: child

	if (!CreateDialogParam(parent->getInstance(), MAKEINTRESOURCE(_dialogId), parent->hWnd(),
		Dialog::_DialogProc, reinterpret_cast<LPARAM>(this)) ) // pass pointer to object
	{
		OutputDebugString(Sprintf(L"ERROR: CreateDialogParam failed, error #%d.\n", GetLastError()).c_str());
		return; // window creation failed
	}
	
	if (_border) {
		SetWindowLongPtr(this->hWnd(), GWL_EXSTYLE,
			GetWindowLongPtr(this->hWnd(), GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
	}

	ShowWindow(this->hWnd(), SW_SHOW);
	this->setPos(nullptr, pos, sz, SWP_NOZORDER);
}

INT_PTR DialogCtrl::dlgProc(UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_NCPAINT:
		if (this->_drawBorders(wp, lp)) return TRUE; // themed borders
		break;
	}
	return Dialog::dlgProc(msg, wp, lp); // forward to parent class message handler
}