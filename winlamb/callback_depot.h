/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <vector>
#include <Windows.h>

namespace winlamb {

template<typename idtypeT, typename paramsT, typename traitsT>
class callback_depot final {
public:
	using depot_callback_type = std::function<typename traitsT::ret_type(paramsT)>;
private:
	struct _unit final {
		idtypeT identifier;
		depot_callback_type callback;
	};
	std::vector<_unit> _units;

public:
	bool empty() const
	{
		return this->_units.empty();
	}

	void add(idtypeT identifier, depot_callback_type callback)
	{
		for (auto& unit : this->_units) {
			if (unit.identifier == identifier) {
				unit.callback = std::move(callback); // replace existing
				return;
			}
		}
		this->_units.push_back({ identifier, std::move(callback) }); // add new
	}

	void add(std::initializer_list<idtypeT> identifiers, depot_callback_type callback)
	{
		idtypeT firstId = *identifiers.begin();
		this->add(firstId, std::move(callback)); // store user callback once
		size_t idxLast = this->_units.size() - 1;

		for (size_t i = 1; i < identifiers.size(); ++i) {
			if (*(identifiers.begin() + i) != firstId) { // avoid overwriting
				this->add(*(identifiers.begin() + i), [this, idxLast](paramsT p)->typename traitsT::ret_type {
					return this->_units[idxLast].callback(p); // store light wrapper to 1st callback
				});
			}
		}
	}

	typename traitsT::ret_type process(HWND hWnd, UINT msg, idtypeT identifier, paramsT p) const
	{
		for (const auto& unit : this->_units) {
			if (unit.identifier == identifier) {
				return unit.callback(p);
			}
		}
		return traitsT::default_proc(hWnd, msg, p.wParam, p.lParam);
	}
};

}//namespace winlamb