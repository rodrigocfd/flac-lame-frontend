/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <functional>
#include <vector>
#include "Window.h"

namespace wolf {

template<WNDPROC DefProcT = DefWindowProc>
class WindowMsgHandler : public Window {
private:
	struct Msg      { UINT msg; std::vector<std::function<LRESULT(WPARAM, LPARAM)>> callbacks; };
	struct MsgCmd   { WORD cmd; std::function<LRESULT()> callback; };
	struct MsgNotif { UINT_PTR idFrom; UINT code; std::function<LRESULT(NMHDR&)> callback; };
	std::vector<Msg>      _msgs;
	std::vector<MsgCmd>   _msgsCmd;
	std::vector<MsgNotif> _msgsNotif;
	bool _loopHasStarted;
public:
	virtual ~WindowMsgHandler() = 0;
	WindowMsgHandler();
	bool onMessage(UINT msg, std::function<LRESULT(WPARAM, LPARAM)> callback);
	bool onCommand(WORD cmd, std::function<LRESULT()> callback);
	bool onNotify(UINT_PTR idFrom, UINT code, std::function<LRESULT(NMHDR&)> callback);
protected:
	LRESULT _processMsg(UINT msg, WPARAM wp, LPARAM lp);
};



template<WNDPROC DefProcT>
WindowMsgHandler<DefProcT>::~WindowMsgHandler()
{
}

template<WNDPROC DefProcT>
WindowMsgHandler<DefProcT>::WindowMsgHandler()
	: _loopHasStarted(false)
{
	this->onMessage(WM_COMMAND, [this](WPARAM wp, LPARAM lp)->LRESULT {
		for (const auto& handler : this->_msgsCmd) {
			if (handler.cmd == LOWORD(wp)) {
				return handler.callback();
			}
		}
		return DefProcT(this->Window::hWnd(), WM_COMMAND, wp, lp);
	});

	this->onMessage(WM_NOTIFY, [this](WPARAM wp, LPARAM lp)->LRESULT {
		NMHDR& nmhdr = *reinterpret_cast<NMHDR*>(lp);
		for (const auto& handler : this->_msgsNotif) {
			if (handler.idFrom == nmhdr.idFrom && handler.code == nmhdr.code) {
				return handler.callback(nmhdr);
			}
		}
		return DefProcT(this->Window::hWnd(), WM_NOTIFY, wp, lp);
	});
}

template<WNDPROC DefProcT>
bool WindowMsgHandler<DefProcT>::onMessage(UINT msg, std::function<LRESULT(WPARAM, LPARAM)> callback)
{
	// Appends a new callback to any message.
	
	if (this->_loopHasStarted) {
		MessageBox(nullptr,
			L"WindowMsgHandler::onMessage\nMethod called after loop started.",
			L"WOLF internal error",
			MB_ICONERROR);
		return false;
	}

	for (auto& m : this->_msgs) {
		if (m.msg == msg) {
			m.callbacks.emplace_back(std::move(callback));
			return true;
		}
	}
	this->_msgs.push_back({ msg,{ std::move(callback) } });
	return true;
}

template<WNDPROC DefProcT>
bool WindowMsgHandler<DefProcT>::onCommand(WORD cmd, std::function<LRESULT()> callback)
{
	// Appends a new callback to a WM_COMMAND message.
	
	if (this->_loopHasStarted) {
		MessageBox(nullptr,
			L"WindowMsgHandler::onCommand\nMethod called after loop started.",
			L"WOLF internal error",
			MB_ICONERROR);
		return false;
	}

	for (auto& c : this->_msgsCmd) {
		if (c.cmd == cmd) {
			c.callback = std::move(callback); // replace existing
			return true;
		}
	}
	this->_msgsCmd.push_back({ cmd, std::move(callback) });
	return true;
}

template<WNDPROC DefProcT>
bool WindowMsgHandler<DefProcT>::onNotify(UINT_PTR idFrom, UINT code, std::function<LRESULT(NMHDR&)> callback)
{
	// Appends a new callback to a WM_NOTIFY message.
	
	if (this->_loopHasStarted) {
		MessageBox(nullptr,
			L"WindowMsgHandler::onNotify\nMethod called after loop started.",
			L"WOLF internal error",
			MB_ICONERROR);
		return false;
	}

	for (auto& n : this->_msgsNotif) {
		if (n.idFrom == idFrom && n.code == code) {
			n.callback = std::move(callback); // replace existing
			return true;
		}
	}
	this->_msgsNotif.push_back({ idFrom, code, std::move(callback) });
	return true;
}

template<WNDPROC DefProcT>
LRESULT WindowMsgHandler<DefProcT>::_processMsg(UINT msg, WPARAM wp, LPARAM lp)
{
	// Main message processing, intended to be called within the window procedure.
	
	this->_loopHasStarted = true; // once the loop starts, no further messages can be added

	for (const auto& handler : this->_msgs) {
		if (handler.msg == msg) {
			LRESULT retVal = 0;
			for (auto cb = handler.callbacks.rbegin(); cb != handler.callbacks.rend(); ++cb) {
				LRESULT handlerRet = (*cb)(wp, lp); // last added handlers (user's) run first
				if (handlerRet) {
					retVal = handlerRet; // return the last non-default returned value
				}
			}
			return retVal;
		}
	}
	return DefProcT(this->Window::hWnd(), msg, wp, lp);
}

}//namespace wolf