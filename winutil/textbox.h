/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <functional>
#include <string>
#include <vector>
#include <Windows.h>

namespace winutil {

class textbox final {
public:
	using func_keyup_type = std::function<void(BYTE)>;
	struct selection final { int start, len; };

private:
	HWND            _hWnd;
	func_keyup_type _onKeyUp;
public:
	textbox()                : _hWnd(nullptr) { }
	textbox(HWND hwnd)       : textbox() { operator=(hwnd); }
	textbox(const textbox&)  = delete;
	textbox(textbox&& other) : textbox() { operator=(std::move(other)); }

	textbox& operator=(HWND hwnd);
	textbox& operator=(const textbox&) = delete;
	textbox& operator=(textbox&& other);

	HWND                      hwnd() const                             { return _hWnd; }
	textbox&                  create(HWND hParent, int id, POINT pos, LONG width)           { return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL); }
	textbox&                  create_password(HWND hParent, int id, POINT pos, LONG width)  { return _create(hParent, id, pos, {width,21}, ES_AUTOHSCROLL | ES_PASSWORD); }
	textbox&                  create_multi_line(HWND hParent, int id, POINT pos, SIZE size) { return _create(hParent, id, pos, size, ES_MULTILINE | ES_WANTRETURN); }
	int                       get_id() const { return GetDlgCtrlID(_hWnd); }
	textbox&                  set_text(const wchar_t* t);
	textbox&                  set_text(const std::wstring& t)          { return set_text(t.c_str()); }
	std::wstring              get_text() const;
	size_t                    get_text_len() const                     { return GetWindowTextLength(_hWnd); }
	std::vector<std::wstring> get_text_lines() const;
	textbox&                  set_selection(selection selec);
	textbox&                  set_selection_all()                      { return set_selection({0, -1}); }
	selection                 get_selection() const;
	textbox&                  replace_selection(const wchar_t* t);
	textbox&                  replace_selection(const std::wstring& t) { return replace_selection(t.c_str()); }
	textbox&                  enable(bool doEnable);
	textbox&                  focus();
	textbox&                  on_keyup(func_keyup_type callback);
private:
	textbox& _create(HWND hParent, int id, POINT pos, SIZE size, DWORD extraStyles);
	static LRESULT CALLBACK _proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR idSubclass, DWORD_PTR refData);
};

}//namespace winutil