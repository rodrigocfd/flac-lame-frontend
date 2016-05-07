/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <string>
#include <vector>
#include <Windows.h>

namespace winutil {

class statusbar final {
private:
	struct part final {
		UINT sizePixels;
		UINT resizeWeight;
	};

private:
	HWND _hWnd;
	std::vector<part> _parts;
	std::vector<int>  _rightEdges;
public:
	statusbar() : _hWnd(nullptr) { }
	statusbar& operator=(const statusbar& sb) = delete;
	statusbar& operator=(statusbar&& sb) = delete;

	statusbar&   create(HWND hParent);
	void         stretch(WPARAM wp, LPARAM lp);
	statusbar&   add_fixed_part(UINT sizePixels);
	statusbar&   add_resizable_part(UINT resizeWeight);
	statusbar&   set_text(const wchar_t *text, size_t iPart);
	statusbar&   set_text(const std::wstring& text, size_t iPart) { return set_text(text.c_str(), iPart); }
	std::wstring get_text(size_t iPart) const;
	statusbar&   set_icon(HICON hIcon, size_t iPart);
private:
	int _get_parent_cx();
};

}//namespace winutil