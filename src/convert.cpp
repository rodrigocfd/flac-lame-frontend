#include <system_error>
#include <windlg/lib.h>
#include "convert.h"
using namespace lib;
using std::optional, std::wstring, std::wstring_view;

wstring convert::iniPath()
{
	return lib::path::exeDir() + L"\\flac-lame-frontend.ini";
}

static DWORD _execCmd(wstring_view cmdLine)
{
	SECURITY_ATTRIBUTES sa = {
		.nLength = sizeof(SECURITY_ATTRIBUTES),
		.bInheritHandle = TRUE,
	};
	STARTUPINFOW si = {
		.cb = sizeof(STARTUPINFOW),
		.dwFlags = STARTF_USESHOWWINDOW,
		.wShowWindow = SW_SHOW,
	};
	PROCESS_INFORMATION pi{};
	DWORD exitCode = 1; // returned by executed program
	wstring cmdLine2{cmdLine}; // https://devblogs.microsoft.com/oldnewthing/20090601-00/?p=18083

	if (!CreateProcessW(nullptr, cmdLine2.data(), &sa, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) [[unlikely]] {
		throw std::system_error(GetLastError(), std::system_category(), "CreateProcess failed");
	}

	WaitForSingleObject(pi.hProcess, INFINITE); // block the thread until the process finishes
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return exitCode;
}

static void _execAndDelete(wstring_view cmdLine, wstring_view srcFile, bool delSrc)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	OutputDebugStringW( lib::str::fmt(L"Run %s\n", cmdLine).c_str() );
	if (delSrc) OutputDebugStringW( lib::str::fmt(L"Del %s\n", srcFile).c_str() );
#endif

	_execCmd(cmdLine);
	if (delSrc) DeleteFileW(srcFile.data());
}

void convert::toWav(wstring_view iniPath, wstring_view srcFile,
	optional<wstring_view> destFolder, bool delSrc)
{
	optional<wstring> finalDestFolder;
	if (destFolder.has_value() && !str::eqI(path::dirFrom(srcFile), destFolder.value())) // different dir from src?
		finalDestFolder.emplace(destFolder.value());
	
	wstring cmdLine;
	if (path::hasExtension(srcFile, {L"mp3"})) {
		auto lamePath = ini::readStr(iniPath, L"Tools", L"lame");
		cmdLine = str::fmt(L"\"%s\" --decode \"%s\"", lamePath, srcFile);
	} else if (path::hasExtension(srcFile, {L"flac"})) {
		auto flacPath = ini::readStr(iniPath, L"Tools", L"flac");
		cmdLine = str::fmt(L"\"%s\" -d \"%s\"", flacPath, srcFile);
		if (finalDestFolder.has_value()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		throw std::logic_error(
			str::toAnsi( str::fmt(L"Not a FLAC/MP3: %s\n", srcFile) ));
	}

	if (finalDestFolder.has_value()) { // different destination folder
		auto destWavPath = path::swapExtension(srcFile, L"wav");
		cmdLine.append( str::fmt(L" \"%s\\%s\"", finalDestFolder.value(), path::fileFrom(destWavPath)) );
	}

	_execAndDelete(cmdLine, srcFile, delSrc);
}

void convert::toFlac(wstring_view iniPath, wstring_view srcFile,
	optional<wstring_view> destFolder, bool delSrc, wstring_view quality)
{
	optional<wstring> finalDestFolder;
	if (destFolder.has_value() && !str::eqI(path::dirFrom(srcFile), destFolder.value())) // different dir from src?
		finalDestFolder.emplace(destFolder.value());

	wstring finalSrcFile{srcFile};

	if (path::hasExtension(finalSrcFile, {L"flac", L"mp3"})) { // needs intermediary WAV conversion
		if (path::hasExtension(finalSrcFile, {L"mp3"})) { // MP3 to FLAC
			toWav(iniPath, finalSrcFile, finalDestFolder, delSrc); // send WAV straight to new folder, if any
		} else if (path::hasExtension(finalSrcFile, {L"flac"})) { // FLAC to FLAC
			toWav(iniPath, finalSrcFile, finalDestFolder, // send WAV straight to new folder, if any
				finalDestFolder.has_value() ? delSrc : true); // if same destination folder, then delete FLAC (will be replaced)
		}

		if (finalDestFolder.has_value()) { // different destination folder
			finalSrcFile = finalDestFolder.value() + L"\\" + path::fileFrom(finalSrcFile);
			finalDestFolder.reset();
		}
		finalSrcFile = path::swapExtension(finalSrcFile, L"wav"); // our source is now a WAV
		delSrc = true; // delete intermediary WAV
	} else if (!path::hasExtension(finalSrcFile, {L"wav"})) {
		throw std::logic_error(
			str::toAnsi( str::fmt(L"Not a FLAC/WAV: %s\n", finalSrcFile) ));
	}

	auto flacPath = ini::readStr(iniPath, L"Tools", L"flac");
	auto cmdLine = str::fmt(L"\"%s\" -%s -V --no-seektable \"%s\"", flacPath, quality, finalSrcFile);

	if (finalDestFolder.has_value()) { // different destination folder
		wstring destFlacPath{finalSrcFile};
		finalSrcFile = path::swapExtension(finalSrcFile, L"flac");
		cmdLine.append( str::fmt(L" -o \"%s\\%s\"", finalDestFolder.value(), path::fileFrom(destFlacPath)) );
	}

	_execAndDelete(cmdLine, finalSrcFile, delSrc);
}

void convert::toMp3(wstring_view iniPath, wstring_view srcFile,
	optional<wstring_view> destFolder, bool delSrc, wstring_view quality, bool isVbr)
{
	optional<wstring> finalDestFolder;
	if (destFolder.has_value() && !str::eqI(path::dirFrom(srcFile), destFolder.value())) // different dir from src?
		finalDestFolder.emplace(destFolder.value());

	wstring finalSrcFile{srcFile};

	if (path::hasExtension(finalSrcFile, {L"flac", L"mp3"})) { // needs intermediary WAV conversion
		if (path::hasExtension(finalSrcFile, {L"flac"})) { // FLAC to MP3
			toWav(iniPath, finalSrcFile, finalDestFolder, delSrc); // send WAV straight to new folder, if any
		} else if (path::hasExtension(finalSrcFile, {L"mp3"})) { // MP3 to MP3
			toWav(iniPath, finalSrcFile, finalDestFolder, // send WAV straight to new folder, if any
				finalDestFolder.has_value() ? delSrc : true); // if same destination folder, then delete MP3 (will be replaced)
		}

		if (finalDestFolder.has_value()) { // different destination folder
			finalSrcFile = finalDestFolder.value() + L"\\" + path::fileFrom(finalSrcFile);
			finalDestFolder.reset();
		}
		finalSrcFile = path::swapExtension(finalSrcFile, L"wav"); // our source is now a WAV
		delSrc = true; // delete intermediary WAV
	} else if (!path::hasExtension(finalSrcFile, {L"wav"})) {
		throw std::logic_error(
			str::toAnsi( str::fmt(L"Not a FLAC/MP3/WAV: %s\n", finalSrcFile) ));
	}

	auto lamePath = ini::readStr(iniPath, L"Tools", L"lame");
	auto cmdLine = str::fmt(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		lamePath, (isVbr ? L"V" : L"b"), quality.data(), finalSrcFile);

	if (finalDestFolder.has_value()) { // different destination folder
		wstring destMp3Path{finalSrcFile};
		finalSrcFile = path::swapExtension(finalSrcFile, L"mp3");
		cmdLine.append( str::fmt(L" \"%s\\%s\"", finalDestFolder.value(), path::fileFrom(destMp3Path)) );
	}

	_execAndDelete(cmdLine, finalSrcFile, delSrc);
}
