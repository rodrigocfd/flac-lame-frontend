
#pragma once
#include <string>
#include <Windows.h>

class Label final {
private:
	HWND _hWnd;
public:
	Label();
	Label(HWND hwnd);
	Label& operator=(HWND hwnd);
	Label& operator=(std::pair<HWND, int> hWndAndCtrlId);

	HWND         hWnd() const;
	Label&       create(HWND hParent, int id, POINT pos, SIZE size);
	Label&       setText(const wchar_t *t);
	Label&       setText(const std::wstring& t);
	std::wstring getText() const;
	Label&       enable(bool doEnable);
};