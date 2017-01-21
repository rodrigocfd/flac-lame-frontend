/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "inventory.h"

namespace wl {
namespace internals {

class i_inventory {
private:
	inventory& _inventory;
protected:
	i_inventory(inventory& inventory) : _inventory(inventory) { }

public:
	void on_message(UINT msg, inventory::funcT func) {
		this->_inventory.add_message(msg, std::move(func));
	}

	void on_message(std::initializer_list<UINT> msgs, inventory::funcT func) {
		this->_inventory.add_message(msgs, std::move(func));
	}

	void on_command(WORD cmd, inventory::funcT func) {
		this->_inventory.add_command(cmd, std::move(func));
	}

	void on_command(std::initializer_list<WORD> cmds, inventory::funcT func) {
		this->_inventory.add_command(cmds, std::move(func));
	}

	void on_notify(UINT_PTR idFrom, UINT code, inventory::funcT func) {
		this->_inventory.add_notify(idFrom, code, std::move(func));
	}

	void on_notify(UINT_PTR idFrom, std::initializer_list<UINT> codes, inventory::funcT func) {
		this->_inventory.add_notify(idFrom, codes, std::move(func));
	}

	void on_notify(std::pair<UINT_PTR, UINT> id, inventory::funcT func) {
		this->_inventory.add_notify(id, std::move(func));
	}

	void on_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> ids, inventory::funcT func) {
		this->_inventory.add_notify(ids, std::move(func));
	}
};

}//namespace internals
}//namespace wl