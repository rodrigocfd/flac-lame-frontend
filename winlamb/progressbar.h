/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "str.h"
#include <CommCtrl.h>

/**
 * base_wnd <-- base_native_control <-- progressbar
 */

namespace wl {

// Wrapper to progressbar control from Common Controls library.
class progressbar final : public base::native_control {
public:
	progressbar& assign(const base::wnd* parent, int controlId) {
		this->native_control::assign(parent, controlId);
		return *this;
	}

	progressbar& create(const base::wnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->native_control::create(parent, controlId, nullptr, pos, size, PROGRESS_CLASS);
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