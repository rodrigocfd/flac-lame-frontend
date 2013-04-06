
#include "Frame.h"
#include <CommCtrl.h>
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(linker, \
  "\"/manifestdependency:type='Win32' " \
  "name='Microsoft.Windows.Common-Controls' " \
  "version='6.0.0.0' " \
  "processorArchitecture='*' " \
  "publicKeyToken='6595b64144ccf1df' " \
  "language='*'\"")

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
	
	if(!CreateWindowEx(droppable, // hWnd is set on WM_NCCREATE
		(LPCWSTR)MAKELONG(atom, 0), caption, dwStyle,
		rc.left + GetSystemMetrics(SM_CXSCREEN) / 2 - cxClient / 2, // center on screen
		rc.top + GetSystemMetrics(SM_CYSCREEN) / 2 - cyClient / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		0, hMenu, hInst, (LPVOID)this) ) // pass pointer to object
	{
		return -1; // window creation failed
	}

	ShowWindow(this->hWnd(), cmdShow);
	UpdateWindow(this->hWnd());

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

LRESULT FrameApp::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return FramePopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}