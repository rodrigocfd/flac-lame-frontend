/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "internals/i_control.h"
#include "internals/native_control.h"
#include "i_hwnd.h"
#include "params.h"
#include "datetime.h"
#include <CommCtrl.h>

namespace wl {

class datetime_picker final :
	public i_hwnd,
	public internals::i_control<datetime_picker>
{
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

private:
	internals::native_control _control;

public:
	datetime_picker() : i_hwnd(_control.wnd()), i_control(this) { }

	datetime_picker& be(const i_hwnd* ctrl)                  { this->_control.be(ctrl); return *this; }
	datetime_picker& be(const i_hwnd* parent, int controlId) { this->_control.be(parent, controlId); return *this; }

	datetime_picker& create(const i_hwnd* parent, int controlId, POINT pos, LONG width) {
		this->_control.create(parent, controlId, nullptr, pos, {width,21}, DATETIMEPICK_CLASS);
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