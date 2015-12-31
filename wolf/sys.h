/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#pragma once
#include <functional>
#include <vector>
#include "Window.h"

namespace wolf {

class Sys final {
public:
	static void         thread(std::function<void()> callback);
	static DWORD        exec(std::wstring cmdLine);
	static std::wstring pathOfExe();
	static std::wstring pathOfDesktop();
	static bool         hasCtrl();
	static bool         hasShift();
	static int          msgBox(const Window *parent, std::wstring title, std::wstring text, UINT uType=0);
	static int          msgBox(HWND hParent, std::wstring title, std::wstring text, UINT uType=0);
	static std::vector<std::wstring> getDroppedFiles(HDROP hDrop);
};

}//namespace wolf