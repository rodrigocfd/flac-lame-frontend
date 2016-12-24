/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <string>
#include <Windows.h>

#ifdef _DEBUG
#define DBG(what) OutputDebugString(what)
#else
#define DBG(what) (what)
#endif


namespace wet {

class base_wnd {
private:
	HWND _hWnd;
public:
	base_wnd()                  : _hWnd(nullptr) { }
	base_wnd(HWND h)            : _hWnd(h) { }
	base_wnd(const base_wnd& w) : _hWnd(w._hWnd) { }
	base_wnd(base_wnd&& w)      : _hWnd(w._hWnd) { w._hWnd = nullptr; }

	base_wnd& operator=(HWND h)            { this->_hWnd = h; return *this; }
	base_wnd& operator=(const base_wnd& w) { this->_hWnd = w._hWnd; return *this; }

	base_wnd& operator=(base_wnd&& w) {
		if (this != &w) {
			this->_hWnd = w._hWnd;
			w._hWnd = nullptr;
		}
		return *this;
	}

	HWND      hwnd() const      { return this->_hWnd; }
	HINSTANCE hinstance() const { return reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(this->_hWnd, GWLP_HINSTANCE)); }
	int       id() const        { return GetDlgCtrlID(this->_hWnd); }
	base_wnd  parent() const    { return GetParent(this->_hWnd); }

protected:
	void _text(const wchar_t* t) const      { SetWindowTextW(this->_hWnd, t); }
	void _text(const std::wstring& t) const { return this->_text(t.c_str()); }

	std::wstring _text() const {
		int txtLen = GetWindowTextLengthW(this->_hWnd);
		std::wstring buf(txtLen + 1, L'\0');
		GetWindowTextW(this->_hWnd, &buf[0], txtLen + 1);
		buf.resize(txtLen);
		return buf;
	}
};

}//namespace wet