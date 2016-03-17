/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <vector>
#include <Windows.h>

namespace winlamb {

template<typename idT, typename callbackT, typename paramsT, typename traitsT>
class callback_depot final {
private:
	struct _unit final {
		idT identifier;
		callbackT callback;
	};
	std::vector<_unit> _units;

public:
	void add(idT identifier, callbackT callback)
	{
		for (auto& unit : _units) {
			if (unit.identifier == identifier) {
				unit.callback = std::move(callback); // replace existing
				return;
			}
		}
		_units.push_back({ identifier, std::move(callback) }); // add new
	}

	void add(std::initializer_list<idT> identifiers, callbackT callback)
	{
		add(*identifiers.begin(), std::move(callback)); // store 1st callback once
		size_t idxLast = _units.size() - 1;

		for (size_t i = 1; i < identifiers.size(); ++i) {
			if (*(identifiers.begin() + i) != *identifiers.begin()) { // avoid overwriting
				add(*(identifiers.begin() + i), [this, idxLast](paramsT p)->typename traitsT::ret_type {
					return _units[idxLast].callback(p); // store light wrapper to 1st callback
				});
			}
		}
	}

	typename traitsT::ret_type process(HWND hWnd, UINT msg, idT identifier, paramsT p) const
	{
		for (const auto& unit : _units) {
			if (unit.identifier == identifier) {
				return unit.callback(p);
			}
		}
		return traitsT::default_proc(hWnd, msg, p.wParam, p.lParam);
	}
};

}//namespace winlamb