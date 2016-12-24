/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#include <vector>
#include <string>

namespace wet {

template<typename valueT>
class hash_map final {
public:
	struct entry final {
		std::wstring name;
		valueT value;
	};

private:
	std::vector<entry> _entries;

public:
	const std::vector<entry>& entries() const {
		return this->_entries;
	}

	entry* get(const std::wstring& name) {
		for (entry& e : this->_entries) { // simple linear search
			if (e.name == name) return &e; // entry can be renamed, and have its value modified
		}
		return nullptr;
	}

	const entry* get(const std::wstring& name) const {
		for (const entry& e : this->_entries) { // simple linear search
			if (e.name == name) return &e;
		}
		return nullptr;
	}

	bool has(const std::wstring& name) const {
		return this->get(name) != nullptr;
	}

	valueT* val(const std::wstring& name) {
		entry* e = this->get(name);
		return e ? &e->value : nullptr;
	}

	const valueT* val(const std::wstring& name) const {
		const entry* e = this->get(name);
		return e ? &e->value : nullptr;
	}

	entry& add(const std::wstring& name, const valueT& value = valueT()) {
		entry* e = this->get(name);
		if (!e) {
			this->_entries.emplace_back(); // only if doesn't exist yet; insert as non-ordered
			e = &this->_entries.back();
			e->name = name;
		}
		e->value = value;
		return *e; // reference to newly added entry
	}

	valueT& operator[](const std::wstring& name) {
		entry* e = this->get(name);
		if (!e) {
			e = &this->add(name); // if entry doesn't exist, will be added
		}
		return e->value;
	}

	hash_map& clear() {
		this->_entries.clear();
		return *this;
	}

	size_t size() const {
		return this->_entries.size();
	}

	hash_map& reserve(size_t amount) {
		this->_entries.reserve(amount);
		return *this;
	}
};

}//namespace wet