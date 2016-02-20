/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <functional>
#include <vector>
#include "wnd.h"

/**
 * wnd_proc
 *  wnd
 */

namespace winlamb {

template<typename traitsT>
class wnd_proc : public wnd {
public:
	typedef std::function<typename traitsT::ret_type(WPARAM, LPARAM)> msg_func_type;

private:
	struct _msg_unit final {
		UINT message;
		msg_func_type callback;
	};
	std::vector<_msg_unit> _msgs;
	bool _loopStarted;

protected:
	wnd_proc() : _loopStarted(false)
	{
	}

public:
	virtual ~wnd_proc() = default;

	void on_message(UINT msg, msg_func_type callback)
	{
		if (!_loopStarted) {
			for (auto& m : _msgs) {
				if (m.message == msg) {
					m.callback = std::move(callback); // replace existing
					return;
				}
			}
			_msgs.push_back({ msg, std::move(callback) }); // add new message handler
		}
	}

	void on_message(std::initializer_list<UINT> msgs, msg_func_type callback)
	{
		on_message(*msgs.begin(), std::move(callback)); // store 1st message once
		size_t m0 = _msgs.size() - 1;

		for (size_t i = 1; i < msgs.size(); ++i) {
			if (*(msgs.begin() + i) != *msgs.begin()) { // avoid overwriting
				on_message(*(msgs.begin() + i), [this, m0](WPARAM wp, LPARAM lp)->typename traitsT::ret_type {
					return _msgs[m0].callback(wp, lp); // store light wrapper to 1st message
				});
			}
		}
	}

protected:
	static typename traitsT::ret_type CALLBACK _process(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		wnd_proc *pSelf = reinterpret_cast<wnd_proc*>(
			traitsT::get_instance_pointer(hWnd, msg, lp) );
		if (pSelf) {
			if (!pSelf->_loopStarted) {
				pSelf->_loopStarted = true; // no more messages can be added
				pSelf->_hWnd = hWnd; // store HWND
			}
			for (const auto& m : pSelf->_msgs) {
				if (m.message == msg) {
					return m.callback(wp, lp);
				}
			}
		}
		return traitsT::default_proc(hWnd, msg, wp, lp);
	}

private:
	wnd::_hWnd;
};

}//namespace winlamb