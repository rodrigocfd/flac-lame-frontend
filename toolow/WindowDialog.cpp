//
// Realm of the dialog windows, those who take a DLGPROC as a procedure and are created through a dialog resource.
// Part of TOOLOW - Thin Object Oriented Layer Over Win32.
// @author Rodrigo Cesar de Freitas Dias
// @see https://github.com/rodrigocfd/toolow
//

#include "WindowDialog.h"
#include "Font.h"

Dialog::~Dialog()
{
}

INT_PTR Dialog::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	return FALSE; // default-most message processing for a dialog
}

INT_PTR CALLBACK Dialog::_DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	Dialog *pSelf; // in run-time, will be a pointer to the derived-most class
	if(msg == WM_INITDIALOG) {
		pSelf = (Dialog*)lp; // passed on CreateDialogParam() or DialogBoxParam()
		SetWindowLongPtr(hdlg, GWLP_USERDATA, (LONG_PTR)pSelf); // store pointer to object into HWND room
		*((Window*)pSelf) = hdlg; // assign hWnd member
	}
	else pSelf = (Dialog*)GetWindowLongPtr(hdlg, GWLP_USERDATA); // from HWND room, zero if not set yet
	return pSelf ? pSelf->msgHandler(msg, wp, lp) : FALSE; // works, since msgHandler() is virtual
}


DialogPopup::~DialogPopup()
{
}

INT_PTR DialogPopup::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	static Font hSysFont; // to be shared among all regular dialog windows

	switch(msg)
	{
	case WM_INITDIALOG:
		if(!hSysFont.hFont()) {
			Font::Info nfof = Font::GetDefaultDialogFontInfo();
			hSysFont.create(nfof);
		}
		hSysFont.applyOnChildren(this->hWnd());
		this->_setWheelHoverBehavior(); // mousewheel working on cursor hovering, not only when control is focused
		break;

	case WM_APP-1:
		this->_handleSendOrPostFunction(lp); // for tunelling a callback from another thread
		break;
	}
	return Dialog::msgHandler(msg, wp, lp); // forward to parent class message handler
}

BOOL DialogPopup::endDialog(INT_PTR nResult)
{
	return EndDialog(this->hWnd(), nResult); // should always be called on the same window thread
}


DialogApp::~DialogApp()
{
}

int DialogApp::run(HINSTANCE hInst, int cmdShow, int dialogId, int iconId, int accelTableId)
{
	InitCommonControls();

	// The HWND will be assigned during WM_INITDIALOG message, and when the
	// function below returns, it will be already set and ready to be used.
	CreateDialogParam(hInst, MAKEINTRESOURCE(dialogId), nullptr,
		Dialog::_DialogProc, (LPARAM)this); // pass pointer to object

	if(iconId) {
		this->sendMessage(WM_SETICON, ICON_SMALL,
			(LPARAM)(HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
		this->sendMessage(WM_SETICON, ICON_BIG,
			(LPARAM)(HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
	}

	ShowWindow(this->hWnd(), cmdShow);

	HACCEL hAccel = nullptr; // accelerators table
	if(accelTableId)
		hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(accelTableId));

	MSG msg = { 0 };
	BOOL ret = 0;
	while((ret = GetMessage(&msg, nullptr, 0, 0)) != 0) {
		if(ret == -1) return -1; // failure
		if(hAccel && TranslateAccelerator(this->hWnd(), hAccel, &msg)) continue;
		if(IsDialogMessage(this->hWnd(), &msg)) continue;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam; // this can be used as program return value
}

INT_PTR DialogApp::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_CLOSE:
		DestroyWindow(this->hWnd());
		return TRUE;
	case WM_NCDESTROY:
		PostQuitMessage(0);
		return TRUE;
	}
	return DialogPopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}


DialogModal::~DialogModal()
{
}

int DialogModal::show(Window *parent, int dialogId, int accelTableId)
{
	return (int)DialogBoxParam(parent->getInstance(),
		MAKEINTRESOURCE(dialogId), parent->hWnd(), Dialog::_DialogProc,
		(LPARAM)this); // pass pointer to class instance
}

INT_PTR DialogModal::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_INITDIALOG:
		{
			RECT rcPop = this->getWindowRect();
			RECT rcParent = this->getParent().getWindowRect(); // all relative to screen
			this->setPos(nullptr,
				rcParent.left + (rcParent.right - rcParent.left) / 2 - (rcPop.right - rcPop.left) / 2,
				rcParent.top + (rcParent.bottom - rcParent.top) / 2 - (rcPop.bottom - rcPop.top) / 2,
				0, 0, SWP_NOZORDER | SWP_NOSIZE); // center dialog on parent
		}
		break;
	case WM_CLOSE:
		this->endDialog(IDOK);
		return TRUE;
	}
	return DialogPopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}


DialogCtrl::~DialogCtrl()
{
}

void DialogCtrl::create(int id, Window *parent, int x, int y, int cx, int cy)
{
	// Dialog styles to be set on the resource editor:
	// - Control: true
	// - Style: child
	// - Visible: true
	CreateDialogParam(parent->getInstance(), MAKEINTRESOURCE(id), parent->hWnd(),
		Dialog::_DialogProc, (LPARAM)this); // pass pointer to object
	this->setPos(nullptr, x, y, cx, cy, SWP_NOZORDER);
}

INT_PTR DialogCtrl::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_NCPAINT:
		if(this->_drawBorders(wp, lp)) // themed borders
			return TRUE;
		break;
	}
	return Dialog::msgHandler(msg, wp, lp); // forward to parent class message handler
}