/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowModal.h"
#include "Str.h"
using namespace wolf;

WindowModal::~WindowModal()
{
}

WindowModal::WindowModal()
{
	this->WindowMsgHandler::onMessage(WM_DESTROY, [this](WPARAM wp, LPARAM lp)->LRESULT {
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ff381396%28v=vs.85%29.aspx
		this->getParent().enable(true); // re-enable parent window
		SetForegroundWindow(this->getParent().hWnd());
		//BringWindowToTop(this->getParent().hWnd());
		return 0;
	});
}

void WindowModal::show(HWND hOwner)
{
	if (this->Window::hWnd()) {
		MessageBox(hOwner,
			L"WindowModal::show\nMethod called twice.",
			L"WOLF internal error",
			MB_ICONERROR);
		return;
	}

	HINSTANCE hInst = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hOwner, GWLP_HINSTANCE));
	DWORD style = WS_POPUP | WS_VISIBLE,
		exStyle = WS_EX_DLGMODALFRAME;

	if (this->setup.dialogId) {
		if (!this->WindowTopLevel::_loadIfTemplate(hInst, this->setup)) return;
		style |= this->_dialogTemplate.style;
		exStyle |= this->_dialogTemplate.exStyle;
	} else {
		style |= WindowTopLevel::_calcStyle(this->setup);
		exStyle |= WindowTopLevel::_calcStyleEx(this->setup);
		if (!WindowTopLevel::_compensateBorders(style, false, this->setup)) return;
	}

	RECT rcOwner = { 0 };
	GetWindowRect(hOwner, &rcOwner);

	// Parent is turned back active during WM_DESTROY processing.
	// http://blogs.msdn.com/b/oldnewthing/archive/2005/02/18/376080.aspx
	EnableWindow(hOwner, FALSE);

	if (!CreateWindowEx(exStyle, MAKEINTATOM(this->_registerClass(hInst)),
		this->setup.title.c_str(), style,
		rcOwner.left + (rcOwner.right - rcOwner.left) / 2 - this->setup.size.cx / 2, // center on parent
		rcOwner.top + (rcOwner.bottom - rcOwner.top) / 2 - this->setup.size.cy / 2,
		this->setup.size.cx, this->setup.size.cy,
		hOwner, nullptr, hInst,
		static_cast<LPVOID>(this)) ) // pass pointer to object, _hWnd is set on WM_NCCREATE
	{
		MessageBox(nullptr,
			Str::format(L"WindowModal::show\n"
				L"CreateWindowEx failed with error %u.", GetLastError()).c_str(),
			L"WOLF internal error",
			MB_ICONERROR);
		EnableWindow(hOwner, TRUE);
		return;
	}

	// A new loop, so caller function is blocked and awaits modal to be closed.
	this->WindowTopLevel::_loop(this->setup);
}

void WindowModal::show(const WindowParent *owner)
{
	return this->show(owner->hWnd());
}

ATOM WindowModal::_registerClass(HINSTANCE hInst)
{
	WNDCLASSEX wc = { 0 };
	wc.cbWndExtra = sizeof(WindowModal*);
	return WindowProc::_registerClass(hInst, wc, this->setup);
}