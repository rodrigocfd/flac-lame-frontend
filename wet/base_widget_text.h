/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include "base_widget.h"

/**
 * base_wnd <-- base_widget <-- base_widget_text
 */

namespace wet {

template<typename derivedT>
class base_widget_text : public base_widget<derivedT> {
public:
	base_widget_text()          = default;
	base_widget_text(HWND hWnd) : base_widget(hWnd) { }

	derivedT& set_text(const std::wstring& text) {
		this->base_wnd::_text(text);
		return *static_cast<derivedT*>(this);
	}

	std::wstring get_text() const {
		return this->base_wnd::_text();
	}

private:
	base_wnd::_text;
};

}//namespace wet