
#pragma once
#include <string>
#include <Windows.h>

class ComboBox final {
private:
	HWND _hWnd;
public:
	ComboBox()          : _hWnd(nullptr) { }
	ComboBox(HWND hwnd) : _hWnd(hwnd) { }
	ComboBox& operator=(HWND hWnd);

	HWND         hWnd() const                { return _hWnd; }
	ComboBox&    create(HWND hParent, int id, POINT pos, int width, bool sorted);
	ComboBox&    enable(bool doEnable);
	ComboBox&    focus();
	size_t       itemCount() const           { return SendMessage(_hWnd, CB_GETCOUNT, 0, 0); }
	ComboBox&    itemRemoveAll();
	ComboBox&    itemSetSelected(size_t i);
	int          itemGetSelected() const     { return static_cast<int>(SendMessage(_hWnd, CB_GETCURSEL, 0, 0)); }
	ComboBox&    itemAdd(std::initializer_list<const wchar_t*> entries);
	ComboBox&    itemAdd(const wchar_t* entries, wchar_t delimiter = L'|');
	std::wstring itemGetText(size_t i) const;
	std::wstring itemGetSelectedText() const { return itemGetText(itemGetSelected()); }
};