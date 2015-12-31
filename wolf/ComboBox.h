/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class ComboBox final : public Window {
public:
	ComboBox();
	ComboBox(HWND hwnd);
	ComboBox(Window&& w);
	ComboBox&    operator=(HWND hwnd);
	ComboBox&    operator=(Window&& w);
	ComboBox&    create(const WindowParent *parent, int id, POINT pos, int width, bool sorted);
	ComboBox&    create(HWND hParent, int id, POINT pos, int width, bool sorted);
	int          itemCount() const;
	ComboBox&    itemRemoveAll();
	ComboBox&    itemSetSelected(int i);
	int          itemGetSelected() const;
	ComboBox&    itemAdd(std::initializer_list<const wchar_t*> entries);
	ComboBox&    itemAdd(const wchar_t* entries, wchar_t delimiter=L'|');
	std::wstring itemGetText(int i) const;
	std::wstring itemGetSelectedText() const;
private:
	Window::getText;
	Window::setText;
};

}//namespace wolf