/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/base_msgs.h"

/**
 * base_wnd <-- base_inventory <-- base_msgs <-- msg_getdlgcode
 */

namespace wl {

// Adds on_getdlgcode() method to handle WM_GETDLGCODE messages.
class msg_getdlgcode : private base::msgs {
public:
	using getdlgcode_funcT = base::depot<BYTE>::funcT;
private:
	base::depot<BYTE> _gdcDepot;

protected:
	explicit msg_getdlgcode(size_t msgsReserve = 0) : _gdcDepot(msgsReserve) {
		this->on_message(WM_GETDLGCODE, [&](params& p)->LONG_PTR {
			getdlgcode_funcT* pFunc = this->_gdcDepot.find(static_cast<BYTE>(p.wParam));
			return pFunc ? (*pFunc)(p) : this->msgs::_proc_unhandled(p);
		});
	}

	void on_getdlgcode(BYTE vkeyCode, getdlgcode_funcT func) {
		this->_gdcDepot.add(vkeyCode, std::move(func));
	}

	void on_getdlgcode(std::initializer_list<BYTE> vkeyCodes, getdlgcode_funcT func) {
		this->_gdcDepot.add(vkeyCodes, std::move(func));
	}
};

}//namespace wl