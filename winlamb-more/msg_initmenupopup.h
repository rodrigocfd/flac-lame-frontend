/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include "../winlamb/base_msgs.h"

/**
 * base_wnd <-- base_inventory <-- base_msgs <-- msg_initmenupopup
 */

namespace wl {

// Adds on_initmenupopup() method to handle WM_INITMENUPOPUP messages.
class msg_initmenupopup : private base::msgs {
public:
	using initmenupopup_funcT = base::depot<WORD>::funcT;
private:
	base::depot<WORD> _impDepot;

protected:
	explicit msg_initmenupopup(size_t msgsReserve = 0) : _impDepot(msgsReserve) {
		this->on_message(WM_INITMENUPOPUP, [&](params& p)->LONG_PTR {
			initmenupopup_funcT* pFunc = this->_impDepot.find(
				GetMenuItemID(reinterpret_cast<HMENU>(p.wParam), 0)); // ID of first menu item
			return pFunc ? (*pFunc)(p) : this->msgs::_proc_unhandled(p);
		});
	}

	void on_initmenupopup(WORD firstMenuItemId, initmenupopup_funcT func) {
		this->_impDepot.add(firstMenuItemId, std::move(func));
	}

	void on_initmenupopup(std::initializer_list<WORD> firstMenuItemIds, initmenupopup_funcT func) {
		this->_impDepot.add(firstMenuItemIds, std::move(func));
	}
};

}//namespace wl