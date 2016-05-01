
#pragma once
#include <string>
#include <vector>
#include <Windows.h>

class StatusBar final {
private:
	struct Part final {
		UINT sizePixels;
		UINT resizeWeight;
	};

private:
	HWND _hWnd;
	std::vector<Part> _parts;
	std::vector<int>  _rightEdges;
public:
	StatusBar() : _hWnd(nullptr) { }
	StatusBar& operator=(const StatusBar& sb) = delete;
	StatusBar& operator=(StatusBar&& sb) = delete;

	StatusBar&   create(HWND hParent);
	void         stretch(WPARAM wp, LPARAM lp);
	StatusBar&   addFixedPart(UINT sizePixels);
	StatusBar&   addResizablePart(UINT resizeWeight);
	StatusBar&   setText(const wchar_t *text, size_t iPart);
	StatusBar&   setText(const std::wstring& text, size_t iPart) { return setText(text.c_str(), iPart); }
	std::wstring getText(size_t iPart) const;
	StatusBar&   setIcon(HICON hIcon, size_t iPart);
private:
	int _getParentCx();
};