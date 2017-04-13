/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_msgs.h"

/**
 * base_wnd <-- base_msgs <-- msg_keydown
 */

namespace wl {

// Adds on_keydown() method to handle WM_KEYDOWN messages.
class msg_keydown : virtual public base::msgs {
public:
	using funcT = base::inventory<BYTE>::funcT;
private:
	base::inventory<BYTE> _kdInventory;

protected:
	explicit msg_keydown(size_t msgsReserve = 0) : _kdInventory(msgsReserve) {
		this->on_message(WM_KEYDOWN, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_kdInventory.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_keydown(BYTE vkeyCode, funcT func) {
		this->_kdInventory.add(vkeyCode, std::move(func));
	}

	void on_keydown(std::initializer_list<BYTE> vkeyCodes, funcT func) {
		this->_kdInventory.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl