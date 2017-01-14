/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <Windows.h>

namespace wl {

template<typename objT>
class i_control {
private:
	objT& _obj;
protected:
	i_control(objT* obj) : _obj(*obj) { }

public:
	int control_id() const {
		return GetDlgCtrlID(this->_obj.hwnd());
	}

	objT& focus() const {
		SetFocus(this->_obj.hwnd());
		return this->_obj;
	}

	bool has_focus() const {
		GetFocus() == this->_obj.hwnd();
	}

	objT& enable(bool doEnable) const {
		EnableWindow(this->_obj.hwnd(), doEnable);
		return this->_obj;
	}

	bool is_enabled() const {
		return IsWindowEnabled(this->_obj.hwnd());
	}
};

}//namespace wl