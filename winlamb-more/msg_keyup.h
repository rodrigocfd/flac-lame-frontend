/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/base_msgs.h"

/**
 * base_wnd <-- base_inventory <-- base_msgs <-- msg_keyup
 */

namespace wl {

// Adds on_keyup() method to handle WM_KEYUP messages.
class msg_keyup : private base::msgs {
public:
	using keyup_funcT = base::depot<BYTE>::funcT;
private:
	base::depot<BYTE> _kuDepot;

protected:
	explicit msg_keyup(size_t msgsReserve = 0) : _kuDepot(msgsReserve) {
		this->on_message(WM_KEYUP, [&](params& p)->LONG_PTR {
			keyup_funcT* pFunc = this->_kuDepot.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->msgs::_proc_unhandled(p);
		});
	}

public:
	void on_keyup(BYTE vkeyCode, keyup_funcT func) {
		this->_kuDepot.add(vkeyCode, std::move(func));
	}

	void on_keyup(std::initializer_list<BYTE> vkeyCodes, keyup_funcT func) {
		this->_kuDepot.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl