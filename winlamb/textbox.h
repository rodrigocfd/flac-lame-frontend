/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_native_control.h"
#include "plus_control.h"
#include "plus_text.h"

namespace wl {

class textbox final : public plus_control<textbox>, public plus_text<textbox> {
public:
	struct selection final {
		int start;
		int len;
	};

private:
	base_native_control _control;

public:
	textbox() : plus_control(this), plus_text(this) { }

	HWND     hwnd() const                    { return this->_control.hwnd(); }
	textbox& be(HWND hWnd)                   { this->_control.be(hWnd); return *this; }
	textbox& be(HWND hParent, int controlId) { this->_control.be(hParent, controlId); return *this; }

	textbox& create(HWND hParent, int controlId, POINT pos, LONG width) {
		return this->_raw_create(hParent, controlId, pos, {width,21}, ES_AUTOHSCROLL);
	}

	textbox& create_password(HWND hParent, int id, POINT pos, LONG width) {
		return this->_raw_create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD);
	}

	textbox& create_multi_line(HWND hParent, int controlId, POINT pos, SIZE size) {
		return this->_raw_create(hParent, controlId, pos, size, ES_MULTILINE | ES_WANTRETURN);
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
	textbox& _raw_create(HWND hParent, int controlId, POINT pos, SIZE size, DWORD extraStyles) {
		this->_control.create(hParent, controlId, nullptr,
			pos, size, L"Edit",
			WS_CHILD | WS_VISIBLE | extraStyles,
			WS_EX_CLIENTEDGE);
		return *this;
	}
};

}//namespace wl