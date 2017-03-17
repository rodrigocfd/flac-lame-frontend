/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "base_styles.h"
#include "base_text.h"

/**
 *             +-- base_native_control <--+
 * base_wnd <--+                          +-- label
 *             +------- base_text <-------+
 */

namespace wl {

// Wrapper to label (static) control.
class label final :
	public base::native_control,
	public base::text<label>
{
public:
	class styler : public base::styles<label> {
	public:
		styler(label* pLabel) : styles(pLabel) { }
	};

	styler style;

	label() : style(this) { }

	label& assign(const base::wnd* parent, int controlId) {
		this->native_control::assign(parent, controlId);
		return *this;
	}

	label& create(const base::wnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		this->native_control::create(parent, controlId, caption, pos, size, L"Static");
		return *this;
	}

	label& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}
};

}//namespace wl