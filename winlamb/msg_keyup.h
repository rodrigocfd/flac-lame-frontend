/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_msgs.h"

/**
 * base_wnd <-- base_msgs <-- msg_keyup
 */

namespace wl {

// Adds on_keyup() method to handle WM_KEYUP messages.
class msg_keyup : virtual public base::msgs {
public:
	using funcT = base::inventory<BYTE>::funcT;
private:
	base::inventory<BYTE> _kuInventory;

protected:
	explicit msg_keyup(size_t msgsReserve = 0) : _kuInventory(msgsReserve) {
		this->on_message(WM_KEYUP, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_kuInventory.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_keyup(BYTE vkeyCode, funcT func) {
		this->_kuInventory.add(vkeyCode, std::move(func));
	}

	void on_keyup(std::initializer_list<BYTE> vkeyCodes, funcT func) {
		this->_kuInventory.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl