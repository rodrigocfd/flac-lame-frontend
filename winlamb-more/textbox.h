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
 * base_wnd <--+                     +-- textbox
 *             +----- textable <-----+
 */

namespace wl {

// Wrapper to textbox (editbox) control.
class textbox final :
	public native_control,
	public textable<textbox>
{
public:
	struct selection final {
		int start;
		int len;
	};

	class styler : public base::styles<textbox> {
	public:
		styler(textbox* pTextbox) : styles(pTextbox) { }

		textbox& password(bool doSet) {
			return this->styles::set_style(doSet, ES_PASSWORD);
		}
	};

	styler style;

	textbox() : style(this) { }

	textbox& assign(HWND hParent, int controlId) {
		this->native_control::assign(hParent, controlId);
		return *this;
	}

	textbox& assign(const base::wnd* parent, int controlId) {
		return this->assign(parent->hwnd(), controlId);
	}

	textbox& create(HWND hParent, int controlId, POINT pos, LONG width) {
		return this->_raw_create(hParent, controlId, pos, {width,21}, ES_AUTOHSCROLL);
	}

	textbox& create(const base::wnd* parent, int controlId, POINT pos, LONG width) {
		return this->create(parent->hwnd(), controlId, pos, width);
	}

	textbox& create_password(HWND hParent, int id, POINT pos, LONG width) {
		return this->_raw_create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD);
	}

	textbox& create_password(const base::wnd* parent, int id, POINT pos, LONG width) {
		return this->create_password(parent->hwnd(), id, pos, width);
	}

	textbox& create_multi_line(HWND hParent, int controlId, POINT pos, SIZE size) {
		return this->_raw_create(hParent, controlId, pos, size, ES_MULTILINE | ES_WANTRETURN);
	}

	textbox& create_multi_line(const base::wnd* parent, int controlId, POINT pos, SIZE size) {
		return this->create_multi_line(parent->hwnd(), controlId, pos, size);
	}

	textbox& focus() {
		SetFocus(this->hwnd());
		return *this;
	}

	textbox& enable(bool doEnable) {
		EnableWindow(this->hwnd(), doEnable);
		return *this;
	}

	textbox& textbox::selection_set(selection selec) {
		SendMessageW(this->hwnd(), EM_SETSEL, selec.start, selec.start + selec.len);
		return *this;
	}

	textbox& selection_set_all() {
		return this->selection_set({0, -1});
	}

	selection selection_get() const {
		int p0 = 0, p1 = 0;
		SendMessageW(this->hwnd(), EM_GETSEL,
			reinterpret_cast<WPARAM>(&p0), reinterpret_cast<LPARAM>(&p1));
		return {p0, p1 - p0}; // start, length
	}

	textbox& selection_replace(const wchar_t* t) {
		SendMessageW(this->hwnd(), EM_REPLACESEL,
			TRUE, reinterpret_cast<LPARAM>(t));
		return *this;
	}

	textbox& selection_replace(const std::wstring& t) {
		return this->selection_replace(t.c_str());
	}

private:
	textbox& _raw_create(HWND hParent, int controlId, POINT pos, SIZE size, DWORD extraStyles) {
		this->native_control::create(hParent, controlId, nullptr, pos, size,
			L"Edit", WS_CHILD | WS_VISIBLE | extraStyles, WS_EX_CLIENTEDGE);
		return *this;
	}
};

}//namespace wl