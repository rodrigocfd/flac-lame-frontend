/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "WindowSubclass.h"
#include "Str.h"
using namespace wolf;

const UINT WindowSubclass::SUBCLASSID = 1;

WindowSubclass::~WindowSubclass()
{
	this->operator=(nullptr);
}

WindowSubclass::WindowSubclass()
{
}

WindowSubclass::WindowSubclass(HWND hwnd)
{
	this->operator=(hwnd);
}

WindowSubclass::WindowSubclass(Window&& w)
{
	this->operator=(std::move(w));
}

WindowSubclass::WindowSubclass(WindowSubclass&& w)
{
	this->operator=(std::move(w));
}

WindowSubclass& WindowSubclass::operator=(HWND hwnd)
{
	this->_clear();
	this->Window::operator=(hwnd);
	this->_setNew();
	return *this;
}

WindowSubclass& WindowSubclass::operator=(Window&& w)
{
	this->_clear();
	this->Window::operator=(std::move(w));
	this->_setNew();
	return *this;   
}

WindowSubclass& WindowSubclass::operator=(WindowSubclass&& w)
{
	this->_clear();
	this->Window::operator=(std::move(w));
	this->_setNew();
	return *this;
}

WindowSubclass& WindowSubclass::create(HWND hParent, const wchar_t *className, const wchar_t *title,
	int id, POINT pos, SIZE size, DWORD style, DWORD exStyle)
{
	if (this->Window::hWnd()) {
		MessageBox(hParent,
			L"WindowSubclass::create\nControl already created.",
			L"WOLF internal error",
			MB_ICONERROR);
	} else {
		HWND hwnd = CreateWindowEx(exStyle, className, title,
			style | WS_CHILD | WS_VISIBLE,
			pos.x, pos.y, size.cx, size.cy,
			hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
			reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
			nullptr);

		if (!hwnd) {
			MessageBox(hParent,
				Str::format(L"WindowSubclass::create\n"
					L"CreateWindowEx failed with error %u.", GetLastError()).c_str(),
				L"WOLF internal error",
				MB_ICONERROR);
		} else {
			this->operator=(hwnd);
		}
	}
	return *this;
}

WindowSubclass& WindowSubclass::create(const Window *parent, const wchar_t *className, const wchar_t *title,
	int id, POINT pos, SIZE size, DWORD style, DWORD exStyle)
{
	return this->create(parent->hWnd(), className, title, id, pos, size, style, exStyle);
}

void WindowSubclass::_clear()
{
	if (this->Window::hWnd()) {
		RemoveWindowSubclass(this->Window::hWnd(), _subclassProc, SUBCLASSID);
	}
}

void WindowSubclass::_setNew()
{
	if (this->Window::hWnd()) {
		if (!SetWindowSubclass(this->Window::hWnd(), _subclassProc, SUBCLASSID, reinterpret_cast<DWORD_PTR>(this))) {
			MessageBox(nullptr,
				Str::format(L"WindowSubclass::_setNew\n"
					L"SetWindowSubclass failed with error %u.", GetLastError()).c_str(),
				L"WOLF internal error",
				MB_ICONERROR);
		}
	}
}

LRESULT CALLBACK WindowSubclass::_subclassProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
	UINT_PTR idSubclass, DWORD_PTR refData)
{
	WindowSubclass *pSelf = reinterpret_cast<WindowSubclass*>(refData);
	LRESULT ret = pSelf->WindowMsgHandler::_processMsg(msg, wp, lp);

	if (msg == WM_NCDESTROY) {
		RemoveWindowSubclass(hwnd, _subclassProc, idSubclass); // http://blogs.msdn.com/b/oldnewthing/archive/2003/11/11/55653.aspx
	}
	return ret;
}