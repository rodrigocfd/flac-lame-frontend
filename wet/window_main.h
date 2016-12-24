/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "window.h"
#include "base_wnd_main.h"
#include "base_wnd_toplevel.h"
#include "wheel_hover.h"

/**
 *             +-- base_wnd_loop <-- base_wnd_thread <-- window <--+
 *             |                                                   |
 * base_wnd <--+----------------- base_wnd_main <------------------+-- window_main
 *             |                                                   |
 *             +---------------- base_wnd_toplevel <---------------+
 */

namespace wet {

struct setup_window_main final : public setup_window {
	HACCEL accelTable;
	setup_window_main() : accelTable(nullptr) { }
};


class window_main :
	public window<setup_window_main>,
	public base_wnd_main,
	public base_wnd_toplevel
{
protected:
	window_main() {
		this->window::setup.wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
		this->window::setup.wndClassEx.style = CS_DBLCLKS;
		this->window::setup.position = { CW_USEDEFAULT, CW_USEDEFAULT };
		this->window::setup.size = { CW_USEDEFAULT, CW_USEDEFAULT };
		this->window::setup.style = WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN | WS_BORDER;

		// Useful styles to add:
		// WS_SIZEBOX resizable window
		// WS_MAXIMIZEBOX adds maximize button
		// WS_MINIMIZEBOX adds minimize button
		// WS_EX_ACCEPTFILES accepts dropped files (extended style, add on exStyle)
	}

public:
	virtual ~window_main() = default;
	window_main& operator=(const window_main&) = delete;

	virtual int run(HINSTANCE hInst, int cmdShow) override {
		InitCommonControls();
		if (!this->window::_register_create(nullptr, hInst)) return -1;

		ShowWindow(this->base_wnd::hwnd(), cmdShow);
		UpdateWindow(this->base_wnd::hwnd());
		wheel_hover::apply_behavior(this->base_wnd::hwnd());
		return this->base_wnd_loop::_msg_loop(this->window::setup.accelTable); // this can be used as program return value
	}

protected:
	LRESULT def_proc(params p) const {
		switch (p.message) {
		case WM_NCDESTROY:
			PostQuitMessage(0);
			return 0;
		}
		return window::def_proc(p);
	}

private:
	base_wnd_loop::_msg_loop;
	window::_register_create;
	window::def_proc;
};

}//namespace wet