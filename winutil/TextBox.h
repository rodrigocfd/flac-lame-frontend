
#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <Windows.h>

class TextBox final {
public:
	typedef std::function<void(BYTE)> KeyUpFunc;
	struct Selection final { int start, len; };

private:
	HWND      _hWnd;
	KeyUpFunc _onKeyUp;
public:
	TextBox()                : _hWnd(nullptr) { }
	TextBox(HWND hwnd)       : TextBox() { operator=(hwnd); }
	TextBox(const TextBox&)  = delete;
	TextBox(TextBox&& other) : TextBox() { operator=(std::move(other)); }

	TextBox& operator=(HWND hwnd);
	TextBox& operator=(const TextBox&) = delete;
	TextBox& operator=(TextBox&& other);

	HWND                      hWnd() const { return _hWnd; }
	TextBox&                  create(HWND hParent, int id, POINT pos, LONG width)         { return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL); }
	TextBox&                  createPassword(HWND hParent, int id, POINT pos, LONG width) { return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD); }
	TextBox&                  createMultiLine(HWND hParent, int id, POINT pos, SIZE size) { return _create(hParent, id, pos, size, ES_MULTILINE | ES_WANTRETURN); }
	TextBox&                  setText(const wchar_t *t);
	TextBox&                  setText(const std::wstring& t) { return setText(t.c_str()); }
	std::wstring              getText() const;
	size_t                    getTextLen() const             { return GetWindowTextLength(_hWnd); }
	std::vector<std::wstring> getTextLines() const;
	TextBox&                  setSelection(int start = 0, int length = -1);
	TextBox&                  setSelection(Selection selec)  { return setSelection(selec.start, selec.len); }
	Selection                 getSelection() const;
	TextBox&                  replaceSelection(const wchar_t *t);
	TextBox&                  replaceSelection(const std::wstring& t) { return replaceSelection(t.c_str()); }
	TextBox&                  enable(bool doEnable);
	TextBox&                  focus();
	TextBox&                  onKeyUp(KeyUpFunc callback);
private:
	TextBox& _create(HWND hParent, int id, POINT pos, SIZE size, DWORD extraStyles);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};