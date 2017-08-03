/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <vector>
#include <string>

namespace wl {

// Associative array for few elements, uses linear search on a non-ordered vector.
template<typename keyT, typename valueT>
class dictionary final {
public:
	struct item final {
		keyT key;
		valueT value;
	};

private:
	std::vector<item> _items;

public:
	const       std::vector<item>& entries() const { return this->_items; }
	size_t      size() const                       { return this->_items.size(); }
	dictionary& clear()                            { this->_items.clear(); return *this; }
	dictionary& reserve(size_t nEntries)           { this->_items.reserve(nEntries); return *this; }

	item* entry(const keyT& key) {
		for (item& e : this->_items) { // simple linear search
			if (e.key == key) return &e; // non-const: item can have its key and value modified
		}
		return nullptr;
	}

	const item* entry(const keyT& key) const {
		return this->entry(key);
	}

	bool has(const keyT& key) const {
		return this->entry(key) != nullptr;
	}

	valueT* val(const std::wstring& name) {
		item* e = this->entry(name);
		return e ? &e->value : nullptr; // directly returns the value
	}

	const valueT* val(const keyT& key) const {
		return this->val(key);
	}

	item& add(const keyT& key, const valueT& value = valueT()) {
		item* e = this->entry(key);
		if (!e) {
			this->_items.emplace_back(); // only if doesn't exist yet; insert as non-ordered
			e = &this->_items.back();
			e->key = key;
		}
		e->value = value;
		return *e; // reference to newly added item
	}

	void remove(const keyT& key) {
		item* it = this->entry(key);
		if (it) {
			this->_items.erase(this->_items.begin() + (it - &this->_items[0]));
		}
	}

	valueT& operator[](const keyT& key) {
		item* e = this->entry(key);
		if (!e) {
			e = &this->add(key); // if item doesn't exist, will be added
		}
		return e->value;
	}
};

template<typename valueT> using dictionary_str = dictionary<std::wstring, valueT>;
using dictionary_str_str = dictionary_str<std::wstring>;

}//namespace wl