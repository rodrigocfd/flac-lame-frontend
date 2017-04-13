/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_msgs.h"

/**
 * base_wnd <-- base_msgs <-- msg_getdlgcode
 */

namespace wl {

// Adds on_getdlgcode() method to handle WM_GETDLGCODE messages.
class msg_getdlgcode : virtual public base::msgs {
public:
	using funcT = base::inventory<BYTE>::funcT;
private:
	base::inventory<BYTE> _gdcInventory;

protected:
	explicit msg_getdlgcode(size_t msgsReserve = 0) : _gdcInventory(msgsReserve) {
		this->on_message(WM_GETDLGCODE, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_gdcInventory.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_getdlgcode(BYTE vkeyCode, funcT func) {
		this->_gdcInventory.add(vkeyCode, std::move(func));
	}

	void on_getdlgcode(std::initializer_list<BYTE> vkeyCodes, funcT func) {
		this->_gdcInventory.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl