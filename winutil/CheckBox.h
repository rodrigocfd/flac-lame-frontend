
#pragma once
#include <utility>
#include <Windows.h>

class CheckBox final {
private:
	HWND _hWnd;
public:
	CheckBox();
	CheckBox(HWND hWnd);
	CheckBox& operator=(HWND hWnd);
	CheckBox& operator=(std::pair<HWND, int> hWndAndCtrlId);

	HWND      hWnd() const;
	CheckBox& enable(bool doEnable);
	CheckBox& create(HWND hParent, int id, const wchar_t *caption, POINT pos, SIZE size);
	CheckBox& focus();
	bool      isChecked();
	void      setCheck(bool checked);
	void      setCheckAndTrigger(bool checked);
};