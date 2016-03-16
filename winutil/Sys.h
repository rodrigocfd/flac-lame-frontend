
#pragma once
#include <functional>
#include <vector>
#include <Windows.h>

struct Sys final {
	static void         thread(std::function<void()> callback);
	static DWORD        exec(std::wstring cmdLine);
	static std::wstring pathOfExe();
	static std::wstring pathOfDesktop();
	static bool         hasCtrl();
	static bool         hasShift();
	static int          msgBox(HWND hParent, std::wstring title, std::wstring text, UINT uType = 0);
	static void         enableXButton(HWND hWnd, bool enable);
	static bool         fontExists(const wchar_t *name);
};