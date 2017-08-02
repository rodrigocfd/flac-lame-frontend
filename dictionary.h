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
	struct entry final {
		keyT key;
		valueT value;
	};

private:
	std::vector<entry> _entries;

public:
	const       std::vector<entry>& entries() const { return this->_entries; }
	size_t      size() const                        { return this->_entries.size(); }
	dictionary& clear()                             { this->_entries.clear(); return *this; }
	dictionary& reserve(size_t nEntries)            { this->_entries.reserve(nEntries); return *this; }

	entry* get(const keyT& key) {
		for (entry& e : this->_entries) { // simple linear search
			if (e.key == key) return &e; // non-const: entry can have its key and value modified
		}
		return nullptr;
	}

	const entry* get(const keyT& key) const {
		for (const entry& e : this->_entries) { // simple linear search
			if (e.key == key) return &e;
		}
		return nullptr;
	}

	bool has(const keyT& key) const {
		return this->get(key) != nullptr;
	}

	valueT* val(const std::wstring& name) {
		entry* e = this->get(name);
		return e ? &e->value : nullptr; // directly returns the value
	}

	const valueT* val(const keyT& key) const {
		const entry* e = this->get(key);
		return e ? &e->value : nullptr;
	}

	entry& add(const keyT& key, const valueT& value = valueT()) {
		entry* e = this->get(key);
		if (!e) {
			this->_entries.emplace_back(); // only if doesn't exist yet; insert as non-ordered
			e = &this->_entries.back();
			e->key = key;
		}
		e->value = value;
		return *e; // reference to newly added entry
	}

	valueT& operator[](const keyT& key) {
		entry* e = this->get(key);
		if (!e) {
			e = &this->add(key); // if entry doesn't exist, will be added
		}
		return e->value;
	}
};

template<typename entryT> using dictionary_str = dictionary<std::wstring, entryT>;
using dictionary_str_str = dictionary_str<std::wstring>;

}//namespace wl