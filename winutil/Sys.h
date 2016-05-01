
#pragma once
#include <functional>
#include <vector>
#include <Windows.h>

struct Sys final {
	static void         thread(std::function<void()> callback);
	static DWORD        exec(std::wstring cmdLine);
	static DWORD        execShell(std::wstring file);
	static std::vector<std::wstring> getCmdLine();
	static std::wstring pathOfExe();
	static std::wstring pathOfDesktop();
	static bool         hasCtrl()  { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	static bool         hasShift() { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
	static int          msgBox(HWND hParent, std::wstring title, std::wstring text, UINT uType = 0);
	static void         setWheelHoverBehavior(HWND hParent);
	static void         enableXButton(HWND hWnd, bool enable);
	static bool         fontExists(const wchar_t *name);
};