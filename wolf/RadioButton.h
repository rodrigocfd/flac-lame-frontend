/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include "WindowParent.h"

namespace wolf {

class RadioButton final : public Window {
public:
	RadioButton();
	RadioButton(HWND hwnd);
	RadioButton(Window&& w);
	RadioButton& operator=(HWND hwnd);
	RadioButton& operator=(Window&& w);
	RadioButton& create(const WindowParent *parent, int id, const wchar_t *caption, bool firstOfGroup, POINT pos, SIZE size);
	RadioButton& create(HWND hParent, int id, const wchar_t *caption, bool firstOfGroup, POINT pos, SIZE size);
	bool         isChecked();
	void         setCheck(bool checked);
	void         setCheckAndTrigger(bool checked);
};

}//namespace wolf