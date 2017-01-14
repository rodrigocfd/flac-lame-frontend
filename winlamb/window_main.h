/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"
#include "base_loop.h"
#include "base_run.h"
#include "base_wheel.h"
#include "i_hwnd.h"
#include "i_inventory.h"
#include "i_text.h"

namespace wl {

struct setup_window_main final : public setup_window {
	HACCEL accelTable;
	setup_window_main() : accelTable(nullptr) { }
};


class window_main :
	public    i_hwnd,
	protected i_inventory,
	protected i_text<window_main>
{
protected:
	setup_window_main setup;
private:
	window _window;

public:
	window_main() :
		i_hwnd(_window.wnd()), i_inventory(_window.inventory), i_text(this), _window(setup)
	{
		this->on_message(WM_NCDESTROY, [](const params& p)->LRESULT {
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
		base_wheel::apply_behavior(this->hwnd());
		return base_loop::msg_loop(this->hwnd(), this->setup.accelTable); // this can be used as program return value
	}

protected:
	void ui_thread(base_threaded::funcT func) const {
		this->_window.threaded.ui_thread(std::move(func));
	}
};

}//namespace wl