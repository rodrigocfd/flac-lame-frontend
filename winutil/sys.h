/**
 * Part of WinUtil - Windows Utilities
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winutil
 */

#pragma once
#include <functional>
#include <vector>
#include <Windows.h>

namespace winutil {

struct sys final {
	static void         thread(std::function<void()> callback);
	static DWORD        exec(std::wstring cmdLine);
	static DWORD        exec_shell(std::wstring file);
	static std::vector<std::wstring> get_cmd_line();
	static std::wstring path_of_exe();
	static std::wstring path_of_desktop();
	static bool         has_ctrl()  { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	static bool         has_shift() { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }
	
	static int  msg_box(HWND hParent, std::wstring title, std::wstring text, UINT uType = 0);
	static bool show_open_file(HWND hWnd, const wchar_t *filter, std::wstring& buf);
	static bool show_open_file(HWND hWnd, const wchar_t *filter, std::vector<std::wstring>& arrBuf);
	static bool show_save_file(HWND hWnd, const wchar_t *filter, std::wstring& buf, const wchar_t *defFile);
	static bool show_choose_folder(HWND hWnd, std::wstring& buf);
	
	static void set_wheel_hover_behavior(HWND hParent);
	static void enable_x_button(HWND hWnd, bool enable);
	static bool font_exists(const wchar_t *name);
};

}//namespace winutil