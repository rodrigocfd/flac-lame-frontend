/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "i_control.h"
#include "i_hwnd.h"
#include "i_text.h"

namespace wl {

class textbox final :
	public i_hwnd,
	public i_control<textbox>,
	public i_text<textbox>
{
public:
	struct selection final {
		int start;
		int len;
	};

private:
	base_native_control _control;

public:
	textbox() : i_hwnd(_control.wnd()), i_control(this), i_text(this) { }

	textbox& be(const i_hwnd* ctrl)                  { this->_control.be(ctrl); return *this; }
	textbox& be(const i_hwnd* parent, int controlId) { this->_control.be(parent, controlId); return *this; }

	textbox& create(const i_hwnd* parent, int controlId, POINT pos, LONG width) {
		return this->_raw_create(parent, controlId, pos, {width,21}, ES_AUTOHSCROLL);
	}

	textbox& create_password(const i_hwnd* parent, int id, POINT pos, LONG width) {
		return this->_raw_create(parent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD);
	}

	textbox& create_multi_line(const i_hwnd* parent, int controlId, POINT pos, SIZE size) {
		return this->_raw_create(parent, controlId, pos, size, ES_MULTILINE | ES_WANTRETURN);
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
		return { p0, p1 - p0 }; // start, length
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
	textbox& _raw_create(const i_hwnd* parent, int controlId, POINT pos, SIZE size, DWORD extraStyles) {
		this->_control.create(parent, controlId, nullptr,
			pos, size, L"Edit",
			WS_CHILD | WS_VISIBLE | extraStyles,
			WS_EX_CLIENTEDGE);
		return *this;
	}
};

}//namespace wl