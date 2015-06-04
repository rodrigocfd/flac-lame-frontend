/*!
 * @file
 * @brief Dialog windows, created from a dialog resource and using DLGPROC.
 * @details Part of WOLF - Win32 Object Lambda Framework.
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowDialog.h"
#include "Str.h"
using namespace wolf;
using namespace wolf::res;

Dialog::~Dialog()
{
}

INT_PTR CALLBACK Dialog::_DialogProc(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
	Dialog *pSelf = nullptr; // in run-time, will be a pointer to the derived-most class
	if (msg == WM_INITDIALOG) {
		pSelf = reinterpret_cast<Dialog*>(lp); // passed on CreateDialogParam() or DialogBoxParam()
		SetWindowLongPtr(hdlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pSelf)); // store pointer to object into HWND room
		*static_cast<Window*>(pSelf) = hdlg; // assign hWnd member
		pSelf->_onInitDialog(); // before user's
	} else {
		pSelf = reinterpret_cast<Dialog*>(GetWindowLongPtr(hdlg, GWLP_USERDATA)); // from HWND room, zero if not set yet
	}
	return pSelf ? pSelf->_processMessage(msg, wp, lp) : FALSE;
}

void Dialog::_internalEvents()
{
	WindowEventDialog::_internalEvents();
}


DialogTopLevel::~DialogTopLevel()
{
}

BOOL DialogTopLevel::endDialog(INT_PTR nResult)
{
	return EndDialog(this->hWnd(), nResult); // should always be called on the same window thread
}

void DialogTopLevel::_onInitDialog()
{
	static Font hSysFont; // to be shared among all regular dialog windows

	if (!hSysFont.hFont()) {
		Font::Info nfof = Font::GetDefaultDialogFontInfo();
		hSysFont.create(nfof);
	}
	hSysFont.applyOnChildren(this->hWnd());

	this->_setWheelHoverBehavior(); // mousewheel working on cursor hovering, not only when control is focused
}

void DialogTopLevel::_internalEvents()
{
	Dialog::_internalEvents();

	this->onMessage(WindowTopLevel::_WM_ORIGTHREAD, [&](WPARAM wp, LPARAM lp)->INT_PTR {
		this->_handleOrigThread(lp); // for tunelling a callback from another thread
		return TRUE;
	});
}


DialogApp::~DialogApp()
{
}

int DialogApp::run(HINSTANCE hInst, int cmdShow)
{
	InitCommonControls();
	this->_internalEvents();
	this->events(); // attach all user event messages

	// The HWND will be assigned during WM_INITDIALOG message, and when the
	// function below returns, it will be already set and ready to be used.
	if (!CreateDialogParam(hInst, MAKEINTRESOURCE(_dialogId), nullptr,
		Dialog::_DialogProc, reinterpret_cast<LPARAM>(this)) ) // pass pointer to object
	{
		str::Dbg(L"ERROR: CreateDialogParam failed, error #%d.\n", GetLastError());
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

	if (mainMenu.hMenu()) SetMenu(this->hWnd(), mainMenu.hMenu());
	ShowWindow(this->hWnd(), cmdShow);

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

void DialogApp::_onInitDialog()
{
	DialogTopLevel::_onInitDialog();
}

void DialogApp::_internalEvents()
{
	DialogTopLevel::_internalEvents();

	this->onMessage(WM_CLOSE, [&](WPARAM wp, LPARAM lp)->INT_PTR {
		DestroyWindow(this->hWnd());
		return TRUE;
	});

	this->onMessage(WM_NCDESTROY, [&](WPARAM wp, LPARAM lp)->INT_PTR {
		PostQuitMessage(0);
		return TRUE;
	});
}


DialogModal::~DialogModal()
{
}

int DialogModal::show(Window *owner)
{
	this->_internalEvents();
	this->events(); // attach all user event messages

	return static_cast<int>(DialogBoxParam(owner->getInstance(),
		MAKEINTRESOURCE(_dialogId), owner->hWnd(), Dialog::_DialogProc,
		reinterpret_cast<LPARAM>(this))); // pass pointer to class instance
}

void DialogModal::_onInitDialog()
{
	DialogTopLevel::_onInitDialog();

	RECT rcPop = this->getWindowRect();
	RECT rcParent = this->getParent().getWindowRect(); // all relative to screen
	this->setPos(nullptr,
		{ rcParent.left + (rcParent.right - rcParent.left)/2 - (rcPop.right - rcPop.left)/2,
			rcParent.top + (rcParent.bottom - rcParent.top)/2 - (rcPop.bottom - rcPop.top)/2 },
		{ 0, 0 } , SWP_NOZORDER | SWP_NOSIZE); // center dialog on parent
}

void DialogModal::_internalEvents()
{
	DialogTopLevel::_internalEvents();

	this->onMessage(WM_CLOSE, [&](WPARAM wp, LPARAM lp)->INT_PTR {
		this->endDialog(IDOK);
		return TRUE;
	});
}


DialogChild::~DialogChild()
{
}

void DialogChild::create(Window *parent, POINT pos, SIZE sz)
{
	// Dialog styles to be set on VS resource editor:
	// - Border: none
	// - Control: true
	// - Style: child

	this->_internalEvents();
	this->events(); // attach all user event messages

	if (!CreateDialogParam(parent->getInstance(), MAKEINTRESOURCE(_dialogId), parent->hWnd(),
		Dialog::_DialogProc, reinterpret_cast<LPARAM>(this)) ) // pass pointer to object
	{
		str::Dbg(L"ERROR: CreateDialogParam failed, error #%d.\n", GetLastError());
		return; // window creation failed
	}
	
	if (_border) {
		SetWindowLongPtr(this->hWnd(), GWL_EXSTYLE,
			GetWindowLongPtr(this->hWnd(), GWL_EXSTYLE) | WS_EX_CLIENTEDGE);
	}

	ShowWindow(this->hWnd(), SW_SHOWDEFAULT);
	this->setPos(nullptr, pos, sz, SWP_NOZORDER);
}

void DialogChild::_onInitDialog()
{
}

void DialogChild::_internalEvents()
{
	Dialog::_internalEvents();

	this->onMessage(WM_NCPAINT, [&](WPARAM wp, LPARAM lp)->INT_PTR {
		return this->_drawThemeBorders(wp, lp) ?
			TRUE : FALSE;
	});
}