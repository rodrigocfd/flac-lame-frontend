/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowParent.h"
#include "Font.h"
#include "Str.h"
using namespace wolf;
using std::wstring;

WindowParent::SetupParent::SetupParent()
	: dialogId(0)
{
}


WindowParent::~WindowParent()
{
}

WindowParent::WindowParent()
	: _hWndCurFocus(nullptr)
{
	this->WindowMsgHandler::onMessage(WM_CREATE, [this](WPARAM wp, LPARAM lp)->LRESULT {
		static Font sysFont; // to be shared among all parent windows
		if (!sysFont.hFont()) {
			Font::Info nfof = Font::getDefaultDialogFontInfo();
			sysFont.create(nfof);
		}
		sysFont.applyOnChildren(this->Window::hWnd());
		SetFocus(GetNextDlgTabItem(this->Window::hWnd(), nullptr, FALSE)); // focus 1st child according to tab order
		return 0;
	});

	this->WindowMsgHandler::onMessage(WM_ACTIVATE, [this](WPARAM wp, LPARAM lp)->LRESULT {
		if (!HIWORD(wp)) { // it not in minimized state
			if (LOWORD(wp) == WA_INACTIVE) {
				this->_hWndCurFocus = GetFocus(); // save currently focused window
			} else {
				SetFocus(this->_hWndCurFocus); // restore focus back
			}
			return 0;
		}
		return DefWindowProc(this->Window::hWnd(), WM_ACTIVATE, wp, lp);
	});
}

Window WindowParent::getChild(int controlId)
{
	return GetDlgItem(this->Window::hWnd(), controlId);
}

Window WindowParent::createChild(const wchar_t *className, const wchar_t *title, int ctrlId,
	POINT pos, SIZE sz, DWORD style, DWORD exStyle)
{
	return CreateWindowEx(exStyle, className, title, style,
		pos.x, pos.y, sz.cx, sz.cy,
		this->Window::hWnd(), reinterpret_cast<HMENU>(static_cast<UINT_PTR>(ctrlId)),
		this->Window::hInst(), nullptr);
}

bool WindowParent::_loadIfTemplate(HINSTANCE hInst, SetupParent& setup)
{
	if (!setup.dialogId) return true; // no template to be loaded

	wstring err;
	if (!this->_dialogTemplate.load(hInst, setup.dialogId, &err)) {
		MessageBox(nullptr,
			Str::format(L"WindowParent::_loadIfTemplate\n"
				L"WindowDialogTemplate::load failed:\n%s", err.c_str()).c_str(),
			L"WOLF internal error",
			MB_ICONERROR);
		return false;
	}

	this->WindowMsgHandler::onMessage(WM_CREATE, [this](WPARAM wp, LPARAM lp)->LRESULT { // here added after user's
		for (auto& child : this->_dialogTemplate.children) {
			this->createChild(child.windowClass.c_str(), child.title.c_str(), child.id,
				child.pos, child.size, child.style, child.exStyle); // create each template child
		}
		return 0;
	});

	return true;
}