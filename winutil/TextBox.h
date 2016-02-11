
#pragma once
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <Windows.h>

class TextBox final {
public:
	typedef std::function<void(BYTE)> KeyUpFunc;

private:
	HWND      _hWnd;
	KeyUpFunc _onKeyUp;
public:
	TextBox();
	TextBox(HWND hwnd);
	TextBox& operator=(HWND hwnd);
	TextBox& operator=(std::pair<HWND, int> hWndAndCtrlId);

	HWND                      hWnd() const;
	TextBox&                  create(HWND hParent, int id, POINT pos, LONG width);
	TextBox&                  createPassword(HWND hParent, int id, POINT pos, LONG width);
	TextBox&                  createMultiLine(HWND hParent, int id, POINT pos, SIZE size);
	TextBox&                  setText(const wchar_t *t);
	TextBox&                  setText(const std::wstring& t);
	std::wstring              getText() const;
	std::vector<std::wstring> getTextLines() const;
	TextBox&                  setSelection(int start = 0, int length = -1);
	std::pair<int, int>       getSelection() const;
	TextBox&                  replaceSelection(const wchar_t *t);
	TextBox&                  replaceSelection(const std::wstring& t);
	TextBox&                  enable(bool doEnable);
	TextBox&                  focus();
	TextBox&                  onKeyUp(KeyUpFunc callback);
private:
	TextBox& _create(HWND hParent, int id, POINT pos, SIZE size, DWORD extraStyles);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp,
		UINT_PTR idSubclass, DWORD_PTR refData);
};