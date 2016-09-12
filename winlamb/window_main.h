/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"
#include "wnd_main.h"
#include "wnd_loop.h"

/**
 *                        +--- wnd_msgs <---+
 *        +-- wnd_proc <--+                 +-- window <--+
 *        |               +-- wnd_thread <--+             |
 * wnd <--+               |                               +-- window_main
 *        |               +--- wnd_main <-----------------+
 *        |                                               |
 *        +------------------- wnd_loop <-----------------+
 */

namespace winlamb {

struct setup_window_main final : public setup_window {
	HACCEL accelTable;
	setup_window_main() : accelTable(nullptr) { }
};


class window_main :
	public window<setup_window_main>,
	public wnd_main<traits_window>,
	public wnd_loop
{
public:
	virtual ~window_main() = default;
	window_main& operator=(const window_main&) = delete;

protected:
	window_main()
	{
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
	int run(HINSTANCE hInst, int cmdShow) override
	{
		InitCommonControls();
		if (!this->window::create(nullptr, hInst)) return -1;

		ShowWindow(this->wnd::hwnd(), cmdShow);
		UpdateWindow(this->wnd::hwnd());

		return this->wnd_loop::_msg_loop(this->window::setup.accelTable); // this can be used as program return value
	}

private:
	window::create;
	wnd_loop::_msg_loop;
};

}//namespace winlamb