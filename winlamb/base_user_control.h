/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_inventory.h"
#include "base_wnd.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")

namespace wl {

class base_user_control final {
private:
	const base_wnd& _wnd;
	LONG_PTR _processedVal;

public:
	base_user_control(const base_wnd& w, base_inventory& inventory, LONG_PTR processedVal) :
		_wnd(w), _processedVal(processedVal)
	{
		inventory.add_message(WM_NCPAINT, [&](const params& p)->LONG_PTR {
			this->_paint_themed_borders(p);
			return this->_processedVal;
		});
	}

private:
	LONG_PTR _paint_themed_borders(const params& p) const {
		LONG_PTR defRet = DefWindowProcW(this->_wnd.hwnd(), WM_NCPAINT, p.wParam, p.lParam); // will make system draw the scrollbar for us

		if ((GetWindowLongPtrW(this->_wnd.hwnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive() && IsAppThemed()) {
			RECT rc = { 0 };
			GetWindowRect(this->_wnd.hwnd(), &rc); // window outmost coordinates, including margins
			ScreenToClient(this->_wnd.hwnd(), reinterpret_cast<POINT*>(&rc));
			ScreenToClient(this->_wnd.hwnd(), reinterpret_cast<POINT*>(&rc.right));
			rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

			RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
			HDC hdc = GetWindowDC(this->_wnd.hwnd());
			HTHEME hTheme = OpenThemeData(this->_wnd.hwnd(), L"LISTVIEW"); // borrow style from listview

			SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
			SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
			SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
			SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

			CloseThemeData(hTheme);
			ReleaseDC(this->_wnd.hwnd(), hdc);
			return 0;
		}

		return defRet;
	}
};

}//namespace wl