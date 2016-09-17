/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <algorithm>
#include <vector>
#include <Windows.h>

namespace winlamb {

template<
	typename idtypeT,
	typename paramsT,
	typename traitsT
>
class callbacks final {
public:
	using callback_type = std::function<typename traitsT::ret_type(paramsT)>;

private:
	struct _unit final {
		idtypeT identifier;
		callback_type callback;

		_unit(idtypeT id, callback_type cb) : identifier(id), callback(cb) { }
		_unit(idtypeT id) : identifier(id) { }
		bool operator<(const _unit& other) const { return this->identifier < other.identifier; }
	};
	std::vector<_unit> _units;

public:
	bool empty() const { return this->_units.empty(); }

	void add(idtypeT identifier, callback_type callback)
	{
		this->_units.emplace(std::lower_bound(
			this->_units.begin(), this->_units.end(), identifier),
			identifier, std::move(callback)); // insert sorted, replace existing
	}

	void add(std::initializer_list<idtypeT> identifiers, callback_type callback)
	{
		idtypeT firstId = *identifiers.begin();
		this->add(firstId, std::move(callback)); // store 1st callback once

		for (size_t i = 1; i < identifiers.size(); ++i) {
			if (*(identifiers.begin() + i) != firstId) { // avoid overwriting
				this->add(*(identifiers.begin() + i), [this, firstId](paramsT p)->typename traitsT::ret_type {
					return this->_find(firstId)->callback(p); // store light wrapper to 1st callback
				});
			}
		}
	}

	typename traitsT::ret_type process(HWND hWnd, UINT msg, idtypeT identifier, paramsT p) const
	{
		std::vector<_unit>::const_iterator found = this->_find(identifier);
		return found != this->_units.end() ?
			found->callback(p) :
			traitsT::default_proc(hWnd, msg, p.wParam, p.lParam);
	}

private:
	typename std::vector<_unit>::const_iterator _find(idtypeT identifier) const
	{
		// http://stackoverflow.com/a/446327
		std::vector<_unit>::const_iterator found = std::lower_bound(
			this->_units.begin(), this->_units.end(), identifier);
		return (found != this->_units.end() && !(identifier < found->identifier)) ?
			found : this->_units.end();
	}
};

}//namespace winlamb