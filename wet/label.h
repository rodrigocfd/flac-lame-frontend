/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget_text.h"

/**
 * base_wnd <-- base_widget <-- base_widget_text <-- label
 */

namespace wet {

class label final : public base_widget_text<label> {
public:
	label()          = default;
	label(HWND hWnd) : base_widget_text(hWnd) { }

	label& operator=(HWND hWnd) {
		return this->base_widget::operator=(hWnd);
	}

	label& create(const base_wnd* parent, int controlId, const wchar_t* caption, POINT pos, SIZE size) {
		return this->base_widget::create(parent, controlId, caption,
			pos, size, L"Static");
	}

private:
	base_widget::create;
};

}//namespace wet