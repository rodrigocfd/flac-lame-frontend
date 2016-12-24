/**
 * Part of WET - WinAPI Easy Toolkit
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wet
 */

#pragma once
#include <functional>
#include <Windows.h>
#include <process.h>
#include "str.h"

namespace wet {

struct sys final {
	static bool has_ctrl()  { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	static bool has_shift() { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }

	static void thread(std::function<void()> callback) {
		// Cheap alternative to std::thread([](){}).detach().
		struct cb_pack final { std::function<void()> cb; };
		cb_pack* pack = new cb_pack{ std::move(callback) };

		HANDLE thandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void* ptr)->unsigned int {
			cb_pack* pPack = reinterpret_cast<cb_pack*>(ptr);
			pPack->cb(); // invoke user callback
			delete pPack;
			_endthreadex(0); // http://www.codeproject.com/Articles/7732/A-class-to-synchronise-thread-completions/
			return 0;
		}, pack, 0, nullptr));

		CloseHandle(thandle);
	}
	
	static std::vector<std::wstring> get_cmd_line() {
		return str::explode_quoted(GetCommandLineW());
	}

	static DWORD exec(std::wstring cmdLine) {
		SECURITY_ATTRIBUTES sa = { 0 };
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		STARTUPINFO si = { 0 };
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;

		PROCESS_INFORMATION pi = { 0 };
		DWORD dwExitCode = 1; // returned by executed program

		// http://blogs.msdn.com/b/oldnewthing/archive/2009/06/01/9673254.aspx
		if (CreateProcessW(nullptr, &cmdLine[0], &sa, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
			WaitForSingleObject(pi.hProcess, INFINITE); // the program flow is stopped here to wait
			GetExitCodeProcess(pi.hProcess, &dwExitCode);
			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
		}
		return dwExitCode;
	}
	
	static DWORD open_file_shell(std::wstring file) {
		return static_cast<DWORD>(reinterpret_cast<INT_PTR>(
			ShellExecuteW(nullptr, L"open", file.c_str(), nullptr, nullptr, SW_SHOWNORMAL) ));
	}
};

}//namespace wet