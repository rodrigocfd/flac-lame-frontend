/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <string>
#include <Windows.h>
#include <CommCtrl.h>

namespace wolf {

class Window {
private:
	HWND _hWnd;
public:
	virtual ~Window() = default;
	Window();
	Window(HWND hwnd);
	Window(const Window& w);
	Window(Window&& w);
	Window&      operator=(HWND hwnd);
	Window&      operator=(const Window& w);
	Window&      operator=(Window&& w);
	HWND         hWnd() const;
	HINSTANCE    hInst() const;
	LRESULT      sendMessage(UINT msg, WPARAM wp, LPARAM lp) const;
	void         postMessage(UINT msg, WPARAM wp, LPARAM lp) const;
	Window       getParent() const;
	std::wstring getText() const;
	void         setText(const std::wstring& text);
	void         setText(const wchar_t *text);
	void         enable(bool doEnable);
};

}//namespace wolf