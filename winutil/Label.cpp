
#include "Label.h"
using std::wstring;

Label::Label()
	: _hWnd(nullptr)
{
}

Label::Label(HWND hwnd)
	: _hWnd(hwnd)
{
}

Label& Label::operator=(HWND hwnd)
{
	_hWnd = hwnd;
	return *this;
}

HWND Label::hWnd() const
{
	return _hWnd;
}

Label& Label::create(HWND hParent, int id, POINT pos, SIZE size)
{
	return operator=( CreateWindowEx(0, L"Static", nullptr,
		WS_CHILD | WS_VISIBLE,
		pos.x, pos.y, size.cx, size.cy,
		hParent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)),
		reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hParent, GWLP_HINSTANCE)),
		nullptr) );
}

Label& Label::setText(const wchar_t *t)
{
	SetWindowText(_hWnd, t);
	return *this;
}

Label& Label::setText(const wstring& t)
{
	return setText(t.c_str());
}

wstring Label::getText() const
{
	int txtLen = GetWindowTextLength(_hWnd);
	wstring buf(txtLen + 1, L'\0');
	GetWindowText(_hWnd, &buf[0], txtLen + 1);
	buf.resize(txtLen);
	return buf;
}

Label& Label::enable(bool doEnable)
{
	EnableWindow(_hWnd, doEnable);
	return *this;
}