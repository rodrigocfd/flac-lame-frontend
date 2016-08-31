/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"
#include "wnd_control.h"

/**
 * wnd <-- wnd_proc<traits_dialog> <-- window <-- window_control
 */

namespace winlamb {

class window_control : public window<>, public wnd_control {
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
			return this->wnd_control::_paint_themed_borders(p.wParam, p.lParam);
		});
	}

public:
	bool create(HWND hParent, int controlId, POINT position, SIZE size) override
	{
		this->window::setup.position = position;
		this->window::setup.size = size;
		this->window::setup.menu = reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId));
		return this->window::create(hParent);
	}

private:
	window::create;
	wnd_control::_paint_themed_borders;
};

}//namespace winlamb
