
#include "Dialog.h"
#include <CommCtrl.h>
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' " \
  "name='Microsoft.Windows.Common-Controls' " \
  "version='6.0.0.0' " \
  "processorArchitecture='*' " \
  "publicKeyToken='6595b64144ccf1df' " \
  "language='*'\"")

DialogApp::~DialogApp()
{
}

int DialogApp::run(HINSTANCE hInst, int cmdShow, int dialogId, int iconId, int accelTableId)
{
	InitCommonControls();

	// The HWND will be assigned during WM_INITDIALOG message, and when the
	// function below returns, it will be already set and ready to be used.
	CreateDialogParam(hInst, MAKEINTRESOURCE(dialogId), 0,
		Dialog::_DialogProc, (LPARAM)this); // pass pointer to object

	if(iconId) {
		this->sendMessage(WM_SETICON, ICON_SMALL,
			(LPARAM)(HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
		this->sendMessage(WM_SETICON, ICON_BIG,
			(LPARAM)(HICON)LoadImage(hInst, MAKEINTRESOURCE(iconId), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
	}

	ShowWindow(this->hWnd(), cmdShow);

	HACCEL hAccel = 0; // accelerators table
	if(accelTableId)
		hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(accelTableId));

	MSG msg = { 0 };
	BOOL ret = 0;
	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
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