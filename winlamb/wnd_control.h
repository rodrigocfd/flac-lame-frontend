/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <vector>
#include "wnd.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")

/**
* wnd <-- wnd_control
*/

namespace winlamb {

class wnd_control : virtual public wnd {
public:
	virtual ~wnd_control() = default;
	virtual bool create(HWND hParent, int controlId, POINT position, SIZE size) = 0;

protected:
	LRESULT _paint_themed_borders(WPARAM wp, LPARAM lp)
	{
		LRESULT defRet = DefWindowProc(this->wnd::hwnd(), WM_NCPAINT, wp, lp); // will make system draw the scrollbar for us

		if ((GetWindowLongPtr(this->wnd::hwnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive() && IsAppThemed()) {
			RECT rc = { 0 };
			GetWindowRect(this->wnd::hwnd(), &rc); // window outmost coordinates, including margins
			ScreenToClient(this->wnd::hwnd(), reinterpret_cast<POINT*>(&rc));
			ScreenToClient(this->wnd::hwnd(), reinterpret_cast<POINT*>(&rc.right));
			rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

			RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
			HDC hdc = GetWindowDC(this->wnd::hwnd());
			HTHEME hTheme = OpenThemeData(this->wnd::hwnd(), TEXT("LISTVIEW")); // borrow style from listview

			SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
			SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
			SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
			SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

			CloseThemeData(hTheme);
			ReleaseDC(this->wnd::hwnd(), hdc);
			return 0;
		}

		return defRet;
	}
};

}//namespace winlamb
