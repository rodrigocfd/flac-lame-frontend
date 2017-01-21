/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/i_inventory.h"
#include "internals/i_text.h"
#include "internals/i_threaded.h"
#include "internals/loop.h"
#include "internals/run.h"
#include "internals/wheel_hover.h"
#include "internals/window.h"
#include "i_hwnd.h"

namespace wl {

namespace internals {
struct setup_window_main final : public setup_window {
	HACCEL accelTable;
	setup_window_main() : accelTable(nullptr) { }
};
}//namespace internals


class window_main :
	public    i_hwnd,
	protected internals::i_inventory,
	protected internals::i_text<window_main>,
	protected internals::i_threaded
{
private:
	internals::window<internals::setup_window_main> _window;
protected:
	internals::setup_window_main& setup;

public:
	window_main() :
		i_hwnd(_window.wnd()),
		i_inventory(_window.inventoryMsg),
		i_text(this),
		i_threaded(_window.threader),
		setup(_window.setup)
	{
		this->on_message(WM_NCDESTROY, [](const params&)->LRESULT {
			PostQuitMessage(0);
			return 0;
		});

		this->setup.wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
		this->setup.wndClassEx.style = CS_DBLCLKS;
		this->setup.position = { CW_USEDEFAULT, CW_USEDEFAULT };
		this->setup.size = { CW_USEDEFAULT, CW_USEDEFAULT };
		this->setup.style = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER;

		// Useful styles to add:
		// WS_SIZEBOX resizable window
		// WS_MAXIMIZEBOX adds maximize button
		// WS_MINIMIZEBOX adds minimize button
		// WS_EX_ACCEPTFILES accepts dropped files (extended style, add on exStyle)
	}

	int run(HINSTANCE hInst, int cmdShow) {
		InitCommonControls();
		if (!this->_window.register_create(nullptr, hInst)) return -1;

		ShowWindow(this->hwnd(), cmdShow);
		UpdateWindow(this->hwnd());
		internals::wheel_hover::apply_behavior(this->hwnd());
		return internals::loop::msg_loop(this->hwnd(), this->setup.accelTable); // this can be used as program return value
	}
};

}//namespace wl