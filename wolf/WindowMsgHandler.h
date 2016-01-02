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
	bool _canAddMsg;
public:
	virtual ~WindowMsgHandler() = 0;
	WindowMsgHandler();
	bool onMessage(UINT msg, std::function<LRESULT(WPARAM, LPARAM)> callback);
	bool onCommand(WORD cmd, std::function<LRESULT()> callback);
	bool onNotify(UINT_PTR idFrom, UINT code, std::function<LRESULT(NMHDR&)> callback);
protected:
	LRESULT _processMsg(UINT msg, WPARAM wp, LPARAM lp);
	static void _errorShout(const wchar_t *text);
	static void _errorShout(DWORD lastError, const wchar_t *src, const wchar_t *func);
};



template<WNDPROC DefProcT>
WindowMsgHandler<DefProcT>::~WindowMsgHandler()
{
}

template<WNDPROC DefProcT>
WindowMsgHandler<DefProcT>::WindowMsgHandler()
	: _canAddMsg(true)
{
	// It's possible to add a handler during WM_NCCREATE/WM_CREATE handling. If so, vector
	// memory reallocation may occur, crashing the program. Pre-allocation prevents such a
	// crash. That's a cheaper/faster solution than using a vector of pointers.
	this->_msgs.reserve(40); // arbitrary value

	this->onMessage(WM_CREATE, [this](WPARAM wp, LPARAM lp)->LRESULT {
		this->_canAddMsg = false; // no messages can be added after WM_CREATE handling
		return 0;
	});

	this->onMessage(WM_COMMAND, [this](WPARAM wp, LPARAM lp)->LRESULT {
		for (auto i = 0U; i < this->_msgsCmd.size(); ++i) {
			if (this->_msgsCmd[i].cmd == LOWORD(wp)) {
				return this->_msgsCmd[i].callback();
			}
		}
		return DefProcT(this->Window::hWnd(), WM_COMMAND, wp, lp);
	});

	this->onMessage(WM_NOTIFY, [this](WPARAM wp, LPARAM lp)->LRESULT {
		NMHDR& nmhdr = *reinterpret_cast<NMHDR*>(lp);
		for (auto i = 0U; i < this->_msgsNotif.size(); ++i) {
			if (this->_msgsNotif[i].idFrom == nmhdr.idFrom
				&& this->_msgsNotif[i].code == nmhdr.code)
			{
				return this->_msgsNotif[i].callback(nmhdr);
			}
		}
		return DefProcT(this->Window::hWnd(), WM_NOTIFY, wp, lp);
	});
}

template<WNDPROC DefProcT>
bool WindowMsgHandler<DefProcT>::onMessage(UINT msg, std::function<LRESULT(WPARAM, LPARAM)> callback)
{
	// Appends a new callback to any message.
	if (!this->_canAddMsg) return false;
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
	if (!this->_canAddMsg) return false;
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
	if (!this->_canAddMsg) return false;
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
	// Ordinary loop is used instead of iterator because new handlers can be added
	// during WM_NCCREATE/WM_CREATE processing, this invalidating the iterators in
	// the middle of the loop.
	for (auto i = this->_msgs.size(); i-- > 0; ) {
		if (this->_msgs[i].msg == msg) {
			LRESULT retVal = 0;
			for (auto j = this->_msgs[i].callbacks.size(); j-- > 0; ) { // last added handlers (user's) run first
				LRESULT handlerRet = this->_msgs[i].callbacks[j](wp, lp);
				if (handlerRet) {
					retVal = handlerRet; // return the last non-default returned value
				}
			}
			return retVal;
		}
	}
	return DefProcT(this->Window::hWnd(), msg, wp, lp);
}

template<WNDPROC DefProcT>
void WindowMsgHandler<DefProcT>::_errorShout(const wchar_t *text)
{
	MessageBox(nullptr, text, L"WOLF internal error", MB_ICONERROR); // shout on ya face!
}

template<WNDPROC DefProcT>
void WindowMsgHandler<DefProcT>::_errorShout(DWORD lastError, const wchar_t *src, const wchar_t *func)
{
	wchar_t buf[160] = { L'\0' };
	wsprintf(buf, L"%s %s failed.\nError %u (0x%X).\n", src, func, lastError, lastError);
	_errorShout(buf);
}

}//namespace wolf