/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "RadioButton.h"
using namespace wolf;

RadioButton::RadioButton()
{
}

RadioButton::RadioButton(HWND hwnd)
	: Window(hwnd)
{
}

RadioButton::RadioButton(Window&& w)
	: Window(std::move(w))
{
}

RadioButton& RadioButton::operator=(HWND hwnd)
{
	this->Window::operator=(hwnd);
	return *this;
}

RadioButton& RadioButton::operator=(Window&& w)
{
	this->Window::operator=(std::move(w));
	return *this;
}

RadioButton& RadioButton::create(const WindowParent *parent, int id, const wchar_t *caption, bool firstOfGroup, POINT pos, SIZE size)
{
	return this->create(parent->hWnd(), id, caption, firstOfGroup, pos, size);
}

RadioButton& RadioButton::create(HWND hParent, int id, const wchar_t *caption, bool firstOfGroup, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, WC_BUTTON, caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | (firstOfGroup ? WS_GROUP : 0),
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
	return *this;
}

bool RadioButton::isChecked()
{
	return this->sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void RadioButton::setCheck(bool checked)
{
	this->sendMessage(BM_SETCHECK,
		checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

void RadioButton::setCheckAndTrigger(bool checked)
{
	this->setCheck(checked);
	if (checked) {
		this->getParent().sendMessage(WM_COMMAND, // emulate user click
			MAKEWPARAM(GetDlgCtrlID(this->hWnd()), 0),
			reinterpret_cast<LPARAM>(this->hWnd()));
	}
}