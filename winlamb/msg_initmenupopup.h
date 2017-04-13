/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_msgs.h"

/**
 * base_wnd <-- base_msgs <-- msg_initmenupopup
 */

namespace wl {

// Adds on_initmenupopup() method to handle WM_INITMENUPOPUP messages.
class msg_initmenupopup : virtual public base::msgs {
public:
	using funcT = base::inventory<WORD>::funcT;
private:
	base::inventory<WORD> _impInventory;

protected:
	explicit msg_initmenupopup(size_t msgsReserve = 0) : _impInventory(msgsReserve) {
		this->on_message(WM_INITMENUPOPUP, [&](params& p)->LONG_PTR {
			funcT* pFunc = this->_impInventory.find(
				GetMenuItemID(reinterpret_cast<HMENU>(p.wParam), 0)); // ID of first menu item
			return pFunc ? (*pFunc)(p) : this->default_proc(p);
		});
	}

public:
	void on_initmenupopup(WORD firstMenuItemId, funcT func) {
		this->_impInventory.add(firstMenuItemId, std::move(func));
	}

	void on_initmenupopup(std::initializer_list<WORD> firstMenuItemIds, funcT func) {
		this->_impInventory.add(firstMenuItemIds, std::move(func));
	}
};

}//namespace wl