/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <string>
#include <Windows.h>

namespace wl {
namespace internals {

template<typename objT>
class i_text {
private:
	objT& _obj;
protected:
	i_text(objT* obj) : _obj(*obj) { }

public:
	objT& set_text(const wchar_t* text) const {
		SetWindowTextW(this->_obj.hwnd(), text);
		return this->_obj;
	}

	objT& set_text(const std::wstring& text) const {
		return this->set_text(text.c_str());
	}

	std::wstring get_text() const {
		std::wstring buf;
		int len = GetWindowTextLengthW(this->_obj.hwnd());
		if (len) {
			buf.resize(len + 1, L'\0');
			GetWindowTextW(this->_obj.hwnd(), &buf[0], len + 1);
			buf.resize(len);
		}
		return buf;
	}
};

}//namespace internals
}//namespace wl