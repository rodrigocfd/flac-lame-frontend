/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class CheckBox final : public Window {
public:
	CheckBox();
	CheckBox(HWND hwnd);
	CheckBox(Window&& w);
	CheckBox& operator=(HWND hwnd);
	CheckBox& operator=(Window&& w);
	CheckBox& create(const WindowParent *parent, int id, const wchar_t *caption, POINT pos, SIZE size);
	CheckBox& create(HWND hParent, int id, const wchar_t *caption, POINT pos, SIZE size);
	bool      isChecked();
	void      setCheck(bool checked);
	void      setCheckAndTrigger(bool checked);
};

}//namespace wolf