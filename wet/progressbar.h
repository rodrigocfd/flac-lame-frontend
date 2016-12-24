/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget.h"
#include <CommCtrl.h>

/**
 * base_wnd <-- base_widget <-- progressbar
 */

namespace wet {

class progressbar final : public base_widget<progressbar> {
public:
	progressbar()          = default;
	progressbar(HWND hWnd) : base_widget(hWnd) { }

	progressbar& operator=(HWND hWnd) {
		return this->base_widget::operator=(hWnd);
	}

	progressbar& create(const base_wnd* parent, int controlId, POINT pos, SIZE size) {
		return this->base_widget::create(parent, controlId, nullptr,
			pos, size, PROGRESS_CLASS);
	}

	progressbar& set_range(int minVal, int maxVal) {
		SendMessageW(this->base_wnd::hwnd(), PBM_SETRANGE, 0, MAKELPARAM(minVal, maxVal));
		return *this;
	}

	progressbar& set_range(int minVal, size_t maxVal) {
		return this->set_range(minVal, static_cast<int>(maxVal));
	}

	progressbar& set_pos(int pos) {
		SendMessageW(this->base_wnd::hwnd(), PBM_SETPOS, pos, 0);
		return *this;
	}

	progressbar& set_pos(size_t pos) { return this->set_pos(static_cast<int>(pos)); }
	progressbar& set_pos(double pos) { return this->set_pos(static_cast<int>(pos + 0.5)); }

	progressbar& set_waiting(bool isWaiting) {
		if (isWaiting) {
			SetWindowLongPtrW(this->base_wnd::hwnd(), GWL_STYLE, // set this on resource editor won't work
				GetWindowLongPtrW(this->base_wnd::hwnd(), GWL_STYLE) | PBS_MARQUEE);
		}
		SendMessageW(this->base_wnd::hwnd(), PBM_SETMARQUEE, static_cast<WPARAM>(isWaiting), 0);

		// http://stackoverflow.com/a/23689663
		if (!isWaiting) {
			SetWindowLongPtrW(this->base_wnd::hwnd(), GWL_STYLE,
				GetWindowLongPtrW(this->base_wnd::hwnd(), GWL_STYLE) & ~PBS_MARQUEE);
		}
		return *this;
	}

	int get_pos() {
		return static_cast<int>(SendMessageW(this->base_wnd::hwnd(), PBM_GETPOS, 0, 0));
	}

private:
	base_wnd::_text;
	base_widget::create;
};

}//namespace wet