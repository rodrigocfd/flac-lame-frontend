/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "window.h"
#include "base_wnd_control.h"

/**
 *             +-- base_wnd_loop <-- base_wnd_thread <-- window <--+
 * base_wnd <--+                                                   +-- window_control
 *             +----------------- base_wnd_control <---------------+
 */

namespace wet {

class window_control :
	public window<>,
	public base_wnd_control
{
protected:
	window_control() {
		this->window::setup.wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		this->window::setup.wndClassEx.style = CS_DBLCLKS;
		this->window::setup.style = CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		// Useful styles to add:
		// WS_TABSTOP will receive focus on Tab key rotation
		// WS_HSCROLL adds horizontal scrollbar
		// WS_VSCROLL adds vertical scrollbar
		// WS_EX_CLIENTEDGE adds border (extended style, add on exStyle)
	}

public:
	virtual ~window_control() = default;
	window_control& operator=(const window_control&) = delete;

	virtual bool create(HWND hParent, int controlId, POINT position, SIZE size) override {
		this->window::setup.position = position;
		this->window::setup.size = size;
		this->window::setup.menu = reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId));

		return this->window::_register_create(hParent);
	}

	virtual bool create(const base_wnd* parent, int controlId, POINT position, SIZE size) override {
		return this->create(parent->hwnd(), controlId, position, size);
	}

protected:
	LRESULT def_proc(params p) const {
		switch (p.message) {
		case WM_NCPAINT:
			return this->base_wnd_control::_paint_themed_borders(p.wParam, p.lParam);
		}
		return window::def_proc(p);
	}

private:
	base_wnd::_text;
	base_wnd_loop::_msg_loop;
	base_wnd_control::_paint_themed_borders;
	window::_register_create;
	window::def_proc;
};

}//namespace wet