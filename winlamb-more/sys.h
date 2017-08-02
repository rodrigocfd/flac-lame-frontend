/**
 * Part of WinLamb - Win32 API Lambda Library - More
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb-more
 */

#pragma once
#include <functional>
#include <Windows.h>
#include <process.h>
#include <Shlobj.h>
#include "str.h"

namespace wl {

// Windows system utilities.
class sys final {
protected:
	sys() = default;

public:
	static bool has_ctrl()  { return (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0; }
	static bool has_shift() { return (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0; }

	static void thread(std::function<void()> func) {
		// Cheap alternative to std::thread([](){ ... }).detach().
		struct cb_pack final { std::function<void()> func; };
		cb_pack* pack = new cb_pack{ std::move(func) };

		HANDLE thandle = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, [](void* ptr)->unsigned int {
			cb_pack* pPack = reinterpret_cast<cb_pack*>(ptr);
			pPack->func(); // invoke user callback
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

	static std::wstring get_desktop_path() {
		wchar_t buf[MAX_PATH] = { L'\0' };
		SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, buf); // won't have trailing backslash
		return buf;
	}

	static std::wstring get_temp_path() {
		wchar_t buf[MAX_PATH + 1] = { L'\0' };
		GetTempPathW(ARRAYSIZE(buf), buf);
		std::wstring ret = buf;
		if (ret.back() == L'\\') ret.resize(ret.length() - 1); // remove trailing backslash, if any
		return ret;
	}

	static std::wstring get_exe_path() {
		wchar_t buf[MAX_PATH + 1] = { L'\0' };
		GetModuleFileNameW(nullptr, buf, ARRAYSIZE(buf)); // full path name
		std::wstring ret = buf;
		ret.resize(ret.find_last_of(L'\\')); // truncate removing EXE filename and trailing backslash
#ifdef _DEBUG
		ret.resize(ret.find_last_of(L'\\')); // bypass "Debug" folder, remove trailing backslash too
#ifdef _WIN64
		ret.resize(ret.find_last_of(L'\\')); // bypass "x64" folder, remove trailing backslash again
#endif
#endif
		return ret;
	}
};

}//namespace wl