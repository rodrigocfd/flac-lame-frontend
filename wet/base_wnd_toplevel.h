/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_wnd.h"

/**
 * base_wnd <-- base_wnd_toplevel
 */

namespace wet {

class base_wnd_toplevel : virtual public base_wnd {
public:
	void set_caption(const wchar_t* t) const {
		this->base_wnd::_text(t);
	}

	void set_caption(const std::wstring& t) const {
		this->base_wnd::_text(t);
	}

	std::wstring get_caption() const {
		return this->base_wnd::_text();
	}

protected:
	void enable_x_button(bool enable) const {
		// Enable/disable the X button to close the window; has no effect on Alt+F4.
		HMENU hMenu = GetSystemMenu(this->base_wnd::hwnd(), FALSE);
		if (hMenu) {
			UINT dwExtra = enable ? MF_ENABLED : (MF_DISABLED | MF_GRAYED);
			EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | dwExtra);
		}
	}

private:
	base_wnd::_text;
};

}//namespace wet