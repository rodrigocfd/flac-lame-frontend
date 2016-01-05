/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "Window.h"
#pragma comment(lib, "Comctl32.lib")
#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' " \
	"name='Microsoft.Windows.Common-Controls' " \
	"version='6.0.0.0' " \
	"processorArchitecture='*' " \
	"publicKeyToken='6595b64144ccf1df' " \
	"language='*'\"")
using namespace wolf;
using std::wstring;

Window::Window()
	: _hWnd(nullptr)
{
}

Window::Window(HWND hwnd)
	: _hWnd(hwnd)
{
}

Window::Window(const Window& w)
	: _hWnd(w._hWnd)
{
}

Window::Window(Window&& w)
	: _hWnd(w._hWnd)
{
	w._hWnd = nullptr;
}

Window& Window::operator=(HWND hwnd)
{
	this->_hWnd = hwnd;
	return *this;
}

Window& Window::operator=(const Window& w)
{
	return this->operator=(w._hWnd);
}

Window& Window::operator=(Window&& w)
{
	std::swap(this->_hWnd, w._hWnd);
	return *this;
}

HWND Window::hWnd() const
{
	return this->_hWnd;
}

HINSTANCE Window::hInst() const
{
	return reinterpret_cast<HINSTANCE>(GetWindowLongPtr(this->_hWnd, GWLP_HINSTANCE));
}

LRESULT Window::sendMessage(UINT msg, WPARAM wp, LPARAM lp) const
{
	return SendMessage(this->_hWnd, msg, wp, lp);
}

void Window::postMessage(UINT msg, WPARAM wp, LPARAM lp) const
{
	PostMessage(this->_hWnd, msg, wp, lp);
}

Window Window::getParent() const
{
	return GetParent(this->_hWnd);
}

wstring Window::getText() const
{
	int txtLen = GetWindowTextLength(this->_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(this->_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

void Window::setText(const wstring& text)
{
	this->setText(text.c_str());
}

void Window::setText(const wchar_t *text)
{
	SetWindowText(this->_hWnd, text);
}

void Window::setFocus()
{
	SetFocus(this->_hWnd);
}

void Window::enable(bool doEnable)
{
	EnableWindow(this->_hWnd, doEnable);
}