
#include "Window.h"
#include <VsStyle.h>
#include <UxTheme.h>
#pragma comment(lib, "UxTheme.lib")

WindowCtrl::~WindowCtrl()
{
}

bool WindowCtrl::_drawBorders(WPARAM wp, LPARAM lp)
{
	// Intended to be called within WM_NCPAINT processing.
	if((GetWindowLongPtr(this->hWnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive())
	{
		DefWindowProc(this->hWnd(), WM_NCPAINT, wp, lp); // this will make system draw the scrollbar for us; days of struggling until this enlightenment

		RECT rc = { 0 };
		this->getWindowRect(&rc); // window outmost coordinates, including margins
		this->screenToClient((POINT*)&rc);
		this->screenToClient((POINT*)&rc.right);
		rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

		RECT rc2; // clipping region; will draw only within this rectangle
		HDC hdc = GetWindowDC(this->hWnd());
		HTHEME hTheme = OpenThemeData(this->hWnd(), L"LISTVIEW"); // borrow style from listview

		SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
		SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
		SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
		SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
		DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

		CloseThemeData(hTheme);
		ReleaseDC(this->hWnd(), hdc);
		return true; // caller should return 0
	}
	return false; // caller should call default window processing
}