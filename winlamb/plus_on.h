/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include "base_inventory.h"

namespace wl {

class plus_on {
private:
	base_inventory& _inventory;

public:
	plus_on(base_inventory& inventory) : _inventory(inventory) { }

	void on_message(UINT msg, base_inventory::funcT func) {
		this->_inventory.add_message(msg, std::move(func));
	}

	void on_message(std::initializer_list<UINT> msgs, base_inventory::funcT func) {
		this->_inventory.add_message(msgs, std::move(func));
	}

	void on_command(WORD cmd, base_inventory::funcT func) {
		this->_inventory.add_command(cmd, std::move(func));
	}

	void on_command(std::initializer_list<WORD> cmds, base_inventory::funcT func) {
		this->_inventory.add_command(cmds, std::move(func));
	}

	void on_notify(UINT_PTR idFrom, UINT code, base_inventory::funcT func) {
		this->_inventory.add_notify(idFrom, code, std::move(func));
	}

	void on_notify(UINT_PTR idFrom, std::initializer_list<UINT> codes, base_inventory::funcT func) {
		this->_inventory.add_notify(idFrom, codes, std::move(func));
	}

	void on_notify(std::pair<UINT_PTR, UINT> id, base_inventory::funcT func) {
		this->_inventory.add_notify(id, std::move(func));
	}

	void on_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> ids, base_inventory::funcT func) {
		this->_inventory.add_notify(ids, std::move(func));
	}
};

}//namespace wl