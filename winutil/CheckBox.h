
#pragma once
#include <utility>
#include <Windows.h>

class CheckBox final {
private:
	HWND _hWnd;
public:
	CheckBox()          : _hWnd(nullptr) { }
	CheckBox(HWND hwnd) : _hWnd(hwnd) { }
	CheckBox& operator=(HWND hwnd);

	HWND      hWnd() const           { return _hWnd; }
	CheckBox& enable(bool doEnable);
	CheckBox& create(HWND hParent, int id, const wchar_t *caption, POINT pos, SIZE size);
	CheckBox& focus();
	bool      isChecked()            { return SendMessage(_hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED; }
	void      setCheck(bool checked) { SendMessage(_hWnd, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0); }
	void      setCheckAndTrigger(bool checked);
};