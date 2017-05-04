/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/native_control.h"
#include "base_styles.h"
#include "textable.h"

/**
 *             +-- native_control <--+
 * base_wnd <--+                     +-- label
 *             +----- textable <-----+
 */

namespace wl {

// Wrapper to label (static) control.
class label final :
	public native_control,
	public textable<label>
{
public:
	class styler : public base::styles<label> {
	public:
		styler(label* pLabel) : styles(pLabel) { }
	};

	styler style;

	label() : style(this) { }

	label& assign(HWND hParent, int controlId) {
		this->native_control::assign(hParent, controlId);
		return *this;
	}

	label& assign(const base::wnd* parent, int controlId) {
		return this->assign(parent->hwnd(), controlId);
	}

	label& create(HWND hParent, int controlId,
		const wchar_t* caption, POINT pos, SIZE size)
	{
		this->native_control::create(hParent, controlId, caption, pos, size, L"Static");
		return *this;
	}

	label& create(const base::wnd* parent, int controlId,
		const wchar_t* caption, POINT pos, SIZE size)
	{
		return this->create(parent->hwnd(), controlId, caption, pos, size);
	}

	label& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}
};

}//namespace wl