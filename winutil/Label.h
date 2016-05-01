
#pragma once
#include <string>
#include <Windows.h>

class Label final {
private:
	HWND _hWnd;
public:
	Label()          : _hWnd(nullptr) { }
	Label(HWND hwnd) : _hWnd(hwnd) { }
	Label& operator=(HWND hwnd);

	HWND         hWnd() const { return _hWnd; }
	Label&       create(HWND hParent, int id, POINT pos, SIZE size);
	Label&       setText(const wchar_t *t);
	Label&       setText(const std::wstring& t) { return setText(t.c_str()); }
	std::wstring getText() const;
	Label&       enable(bool doEnable);
};