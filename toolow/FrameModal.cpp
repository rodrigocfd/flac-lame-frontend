
#include "Frame.h"

FrameModal::~FrameModal()
{
}

int FrameModal::show(Window *parent, ATOM atom, const wchar_t *caption, int cxClient, int cyClient,
	FramePopup::Style::Resize resizable, HACCEL hAccel)
{
	RECT rcP = { 0 };
	parent->getWindowRect(&rcP);

	DWORD dwStyle = WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_VISIBLE | resizable;
	RECT rc = { 0, 0, cxClient, cyClient };
	AdjustWindowRect(&rc, dwStyle, FALSE); // compensate different theme window borders

	CreateWindowEx(WS_EX_DLGMODALFRAME, // hWnd is set on WM_NCCREATE
		(LPCWSTR)MAKELONG(atom, 0), caption, dwStyle,
		(rcP.right - rcP.left) / 2 + rcP.left - cxClient / 2, // center on parent
		(rcP.bottom - rcP.top) / 2 + rcP.top - cyClient / 2,
		rc.right - rc.left, rc.bottom - rc.top,
		parent->hWnd(), 0, parent->getInstance(),
		(LPVOID)this); // pass pointer to object

	// Parent is turned back active durint WM_CLOSE processing.
	// http://blogs.msdn.com/b/oldnewthing/archive/2005/02/18/376080.aspx
	parent->setEnable(false);

	// A new loop, so caller function is blocked and awaits modal to be closed.
	MSG msg = { 0 };
	while(IsWindow(this->hWnd()) && GetMessage(&msg, 0, 0, 0)) {
		if(!(hAccel && TranslateAccelerator(this->hWnd(), hAccel, &msg)) && !IsDialogMessage(this->hWnd(), &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

LRESULT FrameModal::msgHandler(UINT msg, WPARAM wp, LPARAM lp)
{
	switch(msg)
	{
	case WM_CLOSE:
		this->getParent().setEnable(true); // re-enable parent window
		DestroyWindow(this->hWnd());
		break;
	}
	return FramePopup::msgHandler(msg, wp, lp); // forward to parent class message handler
}