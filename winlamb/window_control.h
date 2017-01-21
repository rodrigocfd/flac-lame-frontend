/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/i_control.h"
#include "internals/i_inventory.h"
#include "internals/i_threaded.h"
#include "internals/user_control.h"
#include "internals/window.h"
#include "i_hwnd.h"

namespace wl {

namespace internals {
struct setup_window_control final : public setup_window { };
}//namespace internals


class window_control :
	public    i_hwnd,
	public    internals::i_inventory,
	public    internals::i_control<window_control>,
	protected internals::i_threaded
{
private:
	internals::window<internals::setup_window_control> _window;
	internals::user_control _control;
protected:
	internals::setup_window_control& setup;

public:
	window_control() :
		i_hwnd(_window.wnd()),
		i_inventory(_window.inventoryMsg),
		i_control(this),
		i_threaded(_window.threader),
		setup(_window.setup),
		_control(_window.wnd(), _window.inventoryMsg, 0)
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
};

}//namespace wl