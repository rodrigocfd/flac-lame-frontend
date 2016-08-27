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
 * wnd <-- wnd_proc<traits_window> <-- window <-- window_control
 */

namespace winlamb {

class window_control : public window<> {
public:
	virtual ~window_control() = default;
	window_control& operator=(const window_control&) = delete;

protected:
	window_control()
	{
		this->window::setup.wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		this->window::setup.wndClassEx.style = CS_DBLCLKS;
		this->window::setup.style = CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		// Useful styles to add:
		// WS_TABSTOP will receive focus on Tab key rotation
		// WS_HSCROLL adds horizontal scrollbar
		// WS_VSCROLL adds vertical scrollbar
		// WS_EX_CLIENTEDGE adds border (extended style, add on exStyle)

		this->wnd_proc::on_message(WM_NCPAINT, [this](params p)->LRESULT {
			return this->_paint_themed_borders(p);
		});
	}

public:
	bool create(HWND hParent, int controlId, POINT position, SIZE size)
	{
		this->window::setup.position = position;
		this->window::setup.size = size;
		this->window::setup.menu = reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId));
		return this->window::create(hParent);
	}

private:
	LRESULT _paint_themed_borders(params p)
	{
		LRESULT defRet = DefWindowProc(this->wnd::hwnd(), WM_NCPAINT, p.wParam, p.lParam); // will make system draw the scrollbar for us
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

	window::create;
};

}//namespace winlamb