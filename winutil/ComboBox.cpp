
#include "ComboBox.h"
#include "Str.h"
using std::initializer_list;
using std::vector;
using std::wstring;

ComboBox& ComboBox::operator=(HWND hWnd)
{
	_hWnd = hWnd;
	return *this;
}

ComboBox& ComboBox::create(HWND hParent, int id, POINT pos, int width, bool sorted)
{
	return operator=( CreateWindowEx(0, L"ComboBox", nullptr,
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | (sorted ? CBS_SORT : 0),
		pos.x, pos.y, width, 0,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

ComboBox& ComboBox::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}

ComboBox& ComboBox::focus()
{
	SetFocus(_hWnd);
	return *this;
}

ComboBox& ComboBox::itemRemoveAll()
{
	SendMessage(_hWnd, CB_RESETCONTENT, 0, 0);
	return *this;
}

ComboBox& ComboBox::itemSetSelected(size_t i)
{
	SendMessage(_hWnd, CB_SETCURSEL, i, 0);
	return *this;
}

ComboBox& ComboBox::itemAdd(initializer_list<const wchar_t*> entries)
{
	for (const wchar_t *s : entries) {
		SendMessage(_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
	}
	return *this;
}

ComboBox& ComboBox::itemAdd(const wchar_t* entries, wchar_t delimiter)
{
	wchar_t delim[2] = { delimiter, L'\0' };
	vector<wstring> vals = Str::explode(entries, delim);
	for (const wstring& s : vals) {
		SendMessage(_hWnd, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.c_str()));
	}
	return *this;
}

wstring ComboBox::itemGetText(size_t i) const
{
	size_t txtLen = SendMessage(_hWnd, CB_GETLBTEXTLEN, i, 0);
	wstring buf(txtLen + 1, L'\0');
	SendMessage(_hWnd, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(&buf[0]));
	buf.resize(txtLen);
	return buf;
}