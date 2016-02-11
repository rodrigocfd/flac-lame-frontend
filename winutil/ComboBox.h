
#pragma once
#include <string>
#include <Windows.h>

class ComboBox final {
private:
	HWND _hWnd;
public:
	ComboBox();
	ComboBox(HWND hWnd);
	ComboBox& operator=(HWND hWnd);
	ComboBox& operator=(std::pair<HWND, int> hWndAndCtrlId);

	HWND         hWnd() const;
	ComboBox&    create(HWND hParent, int id, POINT pos, int width, bool sorted);
	ComboBox&    enable(bool doEnable);
	ComboBox&    focus();
	int          itemCount() const;
	ComboBox&    itemRemoveAll();
	ComboBox&    itemSetSelected(int i);
	int          itemGetSelected() const;
	ComboBox&    itemAdd(std::initializer_list<const wchar_t*> entries);
	ComboBox&    itemAdd(const wchar_t* entries, wchar_t delimiter = L'|');
	std::wstring itemGetText(int i) const;
	std::wstring itemGetSelectedText() const;
};