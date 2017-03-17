/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "base_styles.h"
#include "params.h"
#include "datetime.h"

/**
 * base_wnd <-- base_native_control <-- datetime_picker
 */

namespace wl {

// Wrapper to datetime picker control from Common Controls library.
class datetime_picker final : public base::native_control {
public:
	struct notif final {
		NFYDEC(closeup, NMHDR)
		NFYDEC(datetimechange, NMDATETIMECHANGE)
		NFYDEC(dropdown, NMHDR)
		NFYDEC(format, NMDATETIMEFORMAT)
		NFYDEC(formatquery, NMDATETIMEFORMATQUERY)
		NFYDEC(userstring, NMDATETIMESTRING)
		NFYDEC(wmkeydown, NMDATETIMEWMKEYDOWN)
		NFYDEC(killfocus, NMHDR)
		NFYDEC(setfocus, NMHDR)
	protected:
		notif() = default;
	};

	class styler final : public base::styles<datetime_picker> {
	public:
		explicit styler(datetime_picker* pDtp) : styles(pDtp) { }

		datetime_picker& up_down_control(bool doSet) {
			return this->styles::set_style(doSet, DTS_UPDOWN);
		}
	};

	styler style;

	datetime_picker() : style(this) { }

	datetime_picker& assign(const base::wnd* parent, int controlId) {
		this->native_control::assign(parent, controlId);
		return *this;
	}

	datetime_picker& create(const base::wnd* parent, int controlId, POINT pos, LONG width) {
		this->native_control::create(parent, controlId,
			nullptr, pos, {width,21}, DATETIMEPICK_CLASS);
		return *this;
	}

	datetime_picker& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	datetime_picker& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}

	datetime get_time() const {
		SYSTEMTIME st = { 0 };
		DateTime_GetSystemtime(this->hwnd(), &st);
		return st;
	}

	datetime_picker& set_time(const datetime& dt) {
		DateTime_SetSystemtime(this->hwnd(), GDT_VALID, &dt.systemtime());
		return *this;
	}
};

}//namespace wl