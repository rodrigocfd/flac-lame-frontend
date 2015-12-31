/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include "ComboBox.h"
#include "Str.h"
using namespace wolf;
using std::initializer_list;
using std::vector;
using std::wstring;

ComboBox::ComboBox()
{
}

ComboBox::ComboBox(HWND hwnd)
	: Window(hwnd)
{
}

ComboBox::ComboBox(Window&& w)
	: Window(std::move(w))
{
}

ComboBox& ComboBox::operator=(HWND hwnd)
{
	this->Window::operator=(hwnd);
	return *this;
}

ComboBox& ComboBox::operator=(Window&& w)
{
	this->Window::operator=(std::move(w));
	return *this;
}

ComboBox& ComboBox::create(const WindowParent *parent, int id, POINT pos, int width, bool sorted)
{
	return this->create(parent->hWnd(), id, pos, width, sorted);
}

ComboBox& ComboBox::create(HWND hParent, int id, POINT pos, int width, bool sorted)
{
	this->operator=( CreateWindowEx(0, WC_COMBOBOX, nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | (sorted ? CBS_SORT : 0),
		pos.x, pos.y, width, 0,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
	return *this;
}

int ComboBox::itemCount() const
{
	return static_cast<int>(this->sendMessage(CB_GETCOUNT, 0, 0));
}

ComboBox& ComboBox::itemRemoveAll()
{
	this->sendMessage(CB_RESETCONTENT, 0, 0);
	return *this;
}

ComboBox& ComboBox::itemSetSelected(int i)
{
	this->sendMessage(CB_SETCURSEL, i, 0);
	return *this;
}

int ComboBox::itemGetSelected() const
{
	return static_cast<int>(this->sendMessage(CB_GETCURSEL, 0, 0));
}

ComboBox& ComboBox::itemAdd(initializer_list<const wchar_t*> entries)
{
	for (const wchar_t *s : entries) {
		this->sendMessage(CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	}
	return *this;
}

ComboBox& ComboBox::itemAdd(const wchar_t* entries, wchar_t delimiter)
{
	wchar_t delim[2] = { delimiter, L'\0' };
	vector<wstring> vals = Str::explode(entries, delim);
	for (const wstring& s : vals) {
		this->sendMessage(CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str()));
	}
	return *this;
}

wstring ComboBox::itemGetText(int i) const
{
	int txtLen = static_cast<int>(this->sendMessage(CB_GETLBTEXTLEN, i, 0));
	wstring buf(txtLen + 1, L'\0');
	this->sendMessage(CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(txtLen);
	return buf;
}

wstring ComboBox::itemGetSelectedText() const
{
	return this->itemGetText(this->itemGetSelected());
}