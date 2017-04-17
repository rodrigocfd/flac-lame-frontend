/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/base_msgs.h"

/**
 * base_wnd <-- base_inventory <-- base_msgs <-- msg_keydown
 */

namespace wl {

// Adds on_keydown() method to handle WM_KEYDOWN messages.
class msg_keydown : private base::msgs {
public:
	using keydown_funcT = base::depot<BYTE>::funcT;
private:
	base::depot<BYTE> _kdDepot;

protected:
	explicit msg_keydown(size_t msgsReserve = 0) : _kdDepot(msgsReserve) {
		this->on_message(WM_KEYDOWN, [&](params& p)->LONG_PTR {
			keydown_funcT* pFunc = this->_kdDepot.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->msgs::_proc_unhandled(p);
		});
	}

public:
	void on_keydown(BYTE vkeyCode, keydown_funcT func) {
		this->_kdDepot.add(vkeyCode, std::move(func));
	}

	void on_keydown(std::initializer_list<BYTE> vkeyCodes, keydown_funcT func) {
		this->_kdDepot.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl