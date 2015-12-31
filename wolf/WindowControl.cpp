/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowControl.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")
using namespace wolf;

WindowControl::SetupControl::SetupControl()
	: border(true), scrollVert(false), scrollHorz(false), tabStop(true)
{
}


WindowControl::~WindowControl()
{
}

WindowControl::WindowControl()
{
	this->WindowMsgHandler::onMessage(WM_NCPAINT, [this](WPARAM wp, LPARAM lp)->LRESULT {
		LRESULT defRet = DefWindowProc(this->Window::hWnd(), WM_NCPAINT, wp, lp); // will make system draw the scrollbar for us; days of struggling until this enlightenment
		if ((GetWindowLongPtr(this->Window::hWnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive() && IsAppThemed()) {
			RECT rc = { 0 };
			GetWindowRect(this->Window::hWnd(), &rc); // window outmost coordinates, including margins
			ScreenToClient(this->Window::hWnd(), reinterpret_cast<POINT*>(&rc));
			ScreenToClient(this->Window::hWnd(), reinterpret_cast<POINT*>(&rc.right));
			rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

			RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
			HDC hdc = GetWindowDC(this->Window::hWnd());
			HTHEME hTheme = OpenThemeData(this->Window::hWnd(), L"LISTVIEW"); // borrow style from listview

			SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
			SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
			SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
			SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

			CloseThemeData(hTheme);
			ReleaseDC(this->Window::hWnd(), hdc);
			return 0;
		}
		return defRet;
	});
}

void WindowControl::create(HWND hParent, int ctrlId, POINT pos, SIZE sz)
{
	if (this->Window::hWnd()) {
		WindowMsgHandler::_errorShout(L"WindowControl::create called twice.");
	}

	// For children, WS_BORDER gives old, flat drawing; always use WS_EX_CLIENTEDGE.
	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE));
	DWORD style = CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
		(this->setup.scrollVert ? WS_VSCROLL : 0) |
		(this->setup.scrollHorz ? WS_HSCROLL : 0) |
		(this->setup.tabStop ? (WS_TABSTOP | WS_GROUP) : 0);
	DWORD exStyle = this->setup.border ? WS_EX_CLIENTEDGE : 0;

	if (!CreateWindowEx(exStyle, // http://blogs.msdn.com/b/oldnewthing/archive/2004/07/30/201988.aspx
		MAKEINTATOM(this->_registerClass(hInst)), nullptr, style,
		pos.x, pos.y, sz.cx, sz.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ctrlId)),
		hInst, static_cast<LPVOID>(this)) ) // pass pointer to object; hWnd is set on WM_NCCREATE
	{
		WindowMsgHandler::_errorShout(GetLastError(), L"WindowControl::create", L"CreateWindowEx");
	}
}

void WindowControl::create(const WindowParent *parent, int ctrlId, POINT pos, SIZE sz)
{
	return this->create(parent->hWnd(), ctrlId, pos, sz);
}

ATOM WindowControl::_registerClass(HINSTANCE hInst)
{
	WNDCLASSEX wc = { 0 };
	wc.cbWndExtra = sizeof(WindowControl*);
	return WindowProc::_registerClass(hInst, wc, this->setup);
}