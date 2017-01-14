/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "i_control.h"
#include "i_hwnd.h"
#include <CommCtrl.h>

namespace wl {

class progressbar final :
	public i_hwnd,
	public i_control<progressbar>
{
private:
	base_native_control _control;

public:
	progressbar() : i_hwnd(_control.wnd()), i_control(this) { }

	progressbar& be(const i_hwnd* ctrl)                  { this->_control.be(ctrl); return *this; }
	progressbar& be(const i_hwnd* parent, int controlId) { this->_control.be(parent, controlId); return *this; }

	progressbar& create(const i_hwnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->_control.create(parent, controlId, nullptr, pos, size, PROGRESS_CLASS);
		return *this;
	}

    progressbar& set_range(int minVal, int maxVal) {
		SendMessageW(this->hwnd(), PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal));
		return *this;
	}

	progressbar& set_range(int minVal, size_t maxVal) {
		return this->set_range(minVal, static_cast<int>(maxVal));
	}

	progressbar& set_pos(int pos) {
		SendMessageW(this->hwnd(), PBM_SETPOS, pos, 0);
		return *this;
	}

	progressbar& set_pos(size_t pos) { return this->set_pos(static_cast<int>(pos)); }
	progressbar& set_pos(double pos) { return this->set_pos(static_cast<int>(pos + 0.5)); }

	progressbar& set_waiting(bool isWaiting) {
		if (isWaiting) {
			SetWindowLongPtrW(this->hwnd(), GWL_STYLE, // set this on resource editor won't work
				GetWindowLongPtrW(this->hwnd(), GWL_STYLE) | PBS_MARQUEE);
		}
		SendMessageW(this->hwnd(), PBM_SETMARQUEE, static_cast<WPARAM>(isWaiting), 0);

		// http://stackoverflow.com/a/23689663
		if (!isWaiting) {
			SetWindowLongPtrW(this->hwnd(), GWL_STYLE,
				GetWindowLongPtrW(this->hwnd(), GWL_STYLE) & ~PBS_MARQUEE);
		}
		return *this;
	}

	int get_pos() {
		return static_cast<int>(SendMessageW(this->hwnd(), PBM_GETPOS, 0, 0));
	}
};

}//namespace wl