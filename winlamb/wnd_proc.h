/**
 * Part of WinLamb - Windows Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <functional>
#include "wnd.h"
#include "callback_depot.h"

/**
 * wnd_proc
 *  wnd
 */

namespace winlamb {

template<typename traitsT>
class wnd_proc : public wnd {
public:
	struct params {
		WPARAM wParam;
		LPARAM lParam;
	};
	typedef std::function<typename traitsT::ret_type(params)> func_msg_type;

private:
	callback_depot<UINT, func_msg_type, params, traitsT> _callbacks;
	bool _loopStarted;

protected:
	wnd_proc() : _loopStarted(false) { }

public:
	virtual ~wnd_proc() = default;

	void on_message(UINT msg, func_msg_type callback)
	{
		if (!_loopStarted) {
			_callbacks.add(msg, std::move(callback));
		}
	}

	void on_message(std::initializer_list<UINT> msgs, func_msg_type callback)
	{
		if (!_loopStarted) {
			_callbacks.add(msgs, std::move(callback));
		}
	}

protected:
	static typename traitsT::ret_type CALLBACK _process(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		wnd_proc *pSelf = traitsT::get_instance_pointer<wnd_proc>(hWnd, msg, lp);
		if (pSelf) {
			if (!pSelf->_loopStarted) {
				pSelf->_loopStarted = true; // no more messages can be added
				pSelf->_hWnd = hWnd; // store HWND
			}
			return pSelf->_callbacks.process(hWnd, msg, msg, { wp, lp });
		}
		return traitsT::default_proc(hWnd, msg, wp, lp);
	}

private:
	wnd::_hWnd;
};

}//namespace winlamb