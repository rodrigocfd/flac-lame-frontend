/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "CheckBox.h"
using namespace wolf;

CheckBox::CheckBox()
{
}

CheckBox::CheckBox(HWND hwnd)
	: Window(hwnd)
{
}

CheckBox::CheckBox(Window&& w)
	: Window(std::move(w))
{
}

CheckBox& CheckBox::operator=(HWND hwnd)
{
	this->Window::operator=(hwnd);
	return *this;
}

CheckBox& CheckBox::operator=(Window&& w)
{
	this->Window::operator=(std::move(w));
	return *this;
}

CheckBox& CheckBox::create(const WindowParent *parent, int id, const wchar_t *caption, POINT pos, SIZE size)
{
	return this->create(parent->hWnd(), id, caption, pos, size);
}

CheckBox& CheckBox::create(HWND hParent, int id, const wchar_t *caption, POINT pos, SIZE size)
{
	this->operator=( CreateWindowEx(0, WC_BUTTON, caption,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
	return *this;
}

bool CheckBox::isChecked()
{
	return this->sendMessage(BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void CheckBox::setCheck(bool checked)
{
	this->sendMessage(BM_SETCHECK,
		checked ? BST_CHECKED : BST_UNCHECKED, 0);
}

void CheckBox::setCheckAndTrigger(bool checked)
{
	this->setCheck(checked);
	if (checked) {
		this->getParent().sendMessage(WM_COMMAND, // emulate user click
			MAKEWPARAM(GetDlgCtrlID(this->hWnd()), 0),
			reinterpret_cast<LPARAM>(this->hWnd()) );
	}
}