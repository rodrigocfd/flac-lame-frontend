/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowMsgHandler.h"

namespace wolf {

template<WNDPROC DefProcT=DefWindowProc>
class WindowGuiThread : public WindowMsgHandler<DefProcT> {
public:
	virtual ~WindowGuiThread() = 0;
	WindowGuiThread();
protected:
	void guiThread(std::function<void()> callback);
private:
	static const UINT WM_GUITHREAD;
	struct CallbackPack final { std::function<void()> fun; };
};



template<WNDPROC DefProcT>
const UINT WindowGuiThread<DefProcT>::WM_GUITHREAD = WM_APP - 1;

template<WNDPROC DefProcT>
WindowGuiThread<DefProcT>::~WindowGuiThread()
{
}

template<WNDPROC DefProcT>
WindowGuiThread<DefProcT>::WindowGuiThread()
{
	this->WindowMsgHandler::onMessage(WM_GUITHREAD, [this](WPARAM wp, LPARAM lp)->LRESULT {
		CallbackPack *pack = reinterpret_cast<CallbackPack*>(lp);
		pack->fun(); // invoke user callback
		delete pack;
		return 0;
	});
}

template<WNDPROC DefProcT>
void WindowGuiThread<DefProcT>::guiThread(std::function<void()> callback)
{
	// This method is analog to SendMessage, but intended to be called from another thread, so a
	// callback function can, tunelled by wndproc, run in the original thread of the window, thus
	// allowing GUI updates. This avoids the user to deal with a custom WM_ message.
	CallbackPack *pack = new CallbackPack{ std::move(callback) };
	this->sendMessage(WM_GUITHREAD, 0, reinterpret_cast<LPARAM>(pack)); // onto window message queue
}

}//namespace wolf