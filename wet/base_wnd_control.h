/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")

/**
 * base_wnd <-- base_wnd_control
 */

namespace wet {

class base_wnd_control : virtual public base_wnd {
public:
	virtual bool create(HWND hParent, int controlId, POINT position, SIZE size) = 0;
	virtual bool create(const base_wnd* parent, int controlId, POINT position, SIZE size) = 0;

protected:
	base_wnd_control() = default;

	LRESULT _paint_themed_borders(WPARAM wp, LPARAM lp) const {
		LRESULT defRet = DefWindowProcW(this->base_wnd::hwnd(), WM_NCPAINT, wp, lp); // will make system draw the scrollbar for us

		if ((GetWindowLongPtrW(this->base_wnd::hwnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive() && IsAppThemed()) {
			RECT rc = { 0 };
			GetWindowRect(this->base_wnd::hwnd(), &rc); // window outmost coordinates, including margins
			ScreenToClient(this->base_wnd::hwnd(), reinterpret_cast<POINT*>(&rc));
			ScreenToClient(this->base_wnd::hwnd(), reinterpret_cast<POINT*>(&rc.right));
			rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

			RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
			HDC hdc = GetWindowDC(this->base_wnd::hwnd());
			HTHEME hTheme = OpenThemeData(this->base_wnd::hwnd(), L"LISTVIEW"); // borrow style from listview

			SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
			SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
			SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
			SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

			CloseThemeData(hTheme);
			ReleaseDC(this->base_wnd::hwnd(), hdc);
			return 0;
		}

		return defRet;
	}
};

}//namespace wet