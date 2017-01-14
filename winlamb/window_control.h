/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "window.h"
#include "base_user_control.h"
#include "i_control.h"
#include "i_hwnd.h"
#include "i_inventory.h"

namespace wl {

struct setup_window_control final : public setup_window { };


class window_control :
	public i_hwnd,
	public i_inventory,
	public i_control<window_control>
{
protected:
	setup_window_control setup;
private:
	window _window;
	base_user_control _control;

public:
	window_control() :
		i_hwnd(_window.wnd()), i_inventory(_window.inventory), i_control(this),
		_window(setup), _control(_window.wnd(), _window.inventory, 0)
	{
		this->setup.wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		this->setup.wndClassEx.style = CS_DBLCLKS;
		this->setup.style = CS_DBLCLKS | WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

		// Useful styles to add:
		// WS_TABSTOP will receive focus on Tab key rotation
		// WS_HSCROLL adds horizontal scrollbar
		// WS_VSCROLL adds vertical scrollbar
		// WS_EX_CLIENTEDGE adds border (extended style, add on exStyle)
	}

	bool create(const i_hwnd* parent, int controlId, POINT position, SIZE size) {
		this->setup.position = position;
		this->setup.size = size;
		this->setup.menu = reinterpret_cast<HMENU>(static_cast<INT_PTR>(controlId));
		return this->_window.register_create(parent->hwnd());
	}

protected:
	void ui_thread(base_threaded::funcT func) const {
		this->_window.threaded.ui_thread(std::move(func));
	}
};

}//namespace wl