/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/native_control.h"
#include "base_styles.h"
#include "str.h"
#include <CommCtrl.h>

/**
 * base_wnd <-- native_control <-- progressbar
 */

namespace wl {

// Wrapper to progressbar control from Common Controls library.
class progressbar final : public native_control {
public:
	class styler : public base::styles<progressbar> {
	public:
		styler(progressbar* pPb) : styles(pPb) { }

		progressbar& vertical(bool doSet) {
			return this->styles::set_style(doSet, PBS_VERTICAL);
		}

		progressbar& smooth_reverse(bool doSet) {
			return this->styles::set_style(doSet, PBS_SMOOTHREVERSE);
		}
	};

	styler style;

	progressbar() : style(this) { }

	progressbar& assign(HWND hParent, int controlId) {
		this->native_control::assign(hParent, controlId);
		return *this;
	}

	progressbar& assign(const base::wnd* parent, int controlId) {
		return this->assign(parent->hwnd(), controlId);
	}

	progressbar& create(HWND hParent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->native_control::create(hParent, controlId, nullptr, pos, size, PROGRESS_CLASS);
		return *this;
	}

	progressbar& create(const base::wnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		return this->create(parent->hwnd(), controlId, caption, pos, size);
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
			this->style.set_style(true, PBS_MARQUEE); // set this on resource editor won't work
		}
		SendMessageW(this->hwnd(), PBM_SETMARQUEE, static_cast<WPARAM>(isWaiting), 0);

		if (!isWaiting) { // http://stackoverflow.com/a/23689663
			this->style.set_style(false, PBS_MARQUEE);
		}
		return *this;
	}

	int get_pos() {
		return static_cast<int>(SendMessageW(this->hwnd(), PBM_GETPOS, 0, 0));
	}
};

}//namespace wl