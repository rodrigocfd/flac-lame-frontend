/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"
#include <VsStyle.h>
#include <Uxtheme.h>
#pragma comment(lib, "UxTheme.lib")

/**
 * window_control
 *  window
 *   threaded<traits_window>
 *    proc<traits_window>
 *     handle
 */

namespace winlamb {

class window_control : public window<setup_window> {
public:
	virtual ~window_control() = default;
	window_control& operator=(const window_control&) = delete;

	window_control()
	{
		setup.wcx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		setup.wcx.style = CS_DBLCLKS;
		//setup.exStyle = WS_EX_CLIENTEDGE; // border
		setup.style = CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		on_message(WM_NCPAINT, [this](WPARAM wp, LPARAM lp)->LRESULT {
			return _paint_themed_borders(wp, lp);
		});
	}

	bool create(HWND hParent, int controlId, POINT position, SIZE size)
	{
		setup.position = position;
		setup.size = size;
		setup.menu = reinterpret_cast<HMENU>(controlId);
		return create(hParent);
	}

private:
	LRESULT _paint_themed_borders(WPARAM wp, LPARAM lp)
	{
		LRESULT defRet = DefWindowProc(hwnd(), WM_NCPAINT, wp, lp); // will make system draw the scrollbar for us
		if ((GetWindowLongPtr(hwnd(), GWL_EXSTYLE) & WS_EX_CLIENTEDGE) && IsThemeActive() && IsAppThemed()) {
			RECT rc = { 0 };
			GetWindowRect(hwnd(), &rc); // window outmost coordinates, including margins
			ScreenToClient(hwnd(), reinterpret_cast<POINT*>(&rc));
			ScreenToClient(hwnd(), reinterpret_cast<POINT*>(&rc.right));
			rc.left += 2; rc.top += 2; rc.right += 2; rc.bottom += 2; // because it comes up anchored at -2,-2

			RECT rc2 = { 0 }; // clipping region; will draw only within this rectangle
			HDC hdc = GetWindowDC(hwnd());
			HTHEME hTheme = OpenThemeData(hwnd(), L"LISTVIEW"); // borrow style from listview

			SetRect(&rc2, rc.left, rc.top, rc.left + 2, rc.bottom); // draw only the borders to avoid flickering
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed left border
			SetRect(&rc2, rc.left, rc.top, rc.right, rc.top + 2);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed top border
			SetRect(&rc2, rc.right - 2, rc.top, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed right border
			SetRect(&rc2, rc.left, rc.bottom - 2, rc.right, rc.bottom);
			DrawThemeBackground(hTheme, hdc, LVP_LISTGROUP, 0, &rc, &rc2); // draw themed bottom border

			CloseThemeData(hTheme);
			ReleaseDC(hwnd(), hdc);
			return 0;
		}
		return defRet;
	}

	window::create;
};

}//namespace winlamb