
#include "Convert.h"
#include <winlamb/executable.h>
#include <winlamb/file.h>
#include <winlamb/path.h>
using namespace wl;
using std::runtime_error;

void Convert::validate_paths(const file_ini& ini)
{
	if (!ini.sections.has(L"Tools") ||
		!ini.sections[L"Tools"].has(L"lame") ||
		!ini.sections[L"Tools"].has(L"flac") )
	{
		throw runtime_error("INI file doesn't have the right entries.");
	}

	// Search for FLAC and LAME tools.
	if (!file::util::exists(ini[L"Tools"][L"lame"])) {
		throw runtime_error(str::to_ascii(
			str::format(L"Could not find LAME tool at:\n%s",
				ini[L"Tools"][L"lame"]) ));
	}
	if (!file::util::exists(ini[L"Tools"][L"flac"])) {
		throw runtime_error(str::to_ascii(
			str::format(L"Could not find FLAC tool at:\n%s",
				ini[L"Tools"][L"lame"]) ));
	}
}

void Convert::to_wav(const file_ini& ini, wstring src, wstring dest, bool delSrc)
{
	_validate_dest_folder(dest);

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	wstring cmdLine;
	if (path::has_extension(src, L".mp3")) {
		cmdLine = str::format(L"\"%s\" --decode \"%s\"",
			ini[L"Tools"][L"lame"], src);
	} else if (path::has_extension(src, L".flac")) {
		cmdLine = str::format(L"\"%s\" -d \"%s\"",
			ini[L"Tools"][L"flac"], src);
		if (!dest.empty()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		throw runtime_error(str::to_ascii(
			str::format(L"Not a FLAC/MP3: %s\n", src) ));
	}

	if (!dest.empty()) { // different destination folder
		wstring destWavPath = src;
		path::change_extension(destWavPath, L".wav");
		cmdLine.append( str::format(L" \"%s\\%s\"",
			dest, path::file_from(destWavPath)) );
	}

	_execute(cmdLine, src, delSrc);
}

void Convert::to_flac(const file_ini& ini, wstring src, wstring dest,
	bool delSrc, const wstring& quality)
{
	_validate_dest_folder(dest);

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (path::has_extension(src, {L".flac", L".mp3"})) { // needs intermediary WAV conversion
		if (path::has_extension(src, L".mp3")) {
			to_wav(ini, src, dest, delSrc); // send WAV straight to new folder, if any
		} else if (path::has_extension(src, L".flac")) {
			to_wav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc); // if same destination folder, then delete source (will be replaced)
		}

		if (!dest.empty()) { // different destination folder
			src = str::format(L"%s\\%s", dest, path::file_from(src));
			dest.clear();
		}

		path::change_extension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!path::has_extension(src, L".wav")) {
		throw runtime_error(str::to_ascii(
			str::format(L"Not a FLAC/WAV: %s\n", src) ));
	}

	wstring cmdLine = str::format(L"\"%s\" -%s -V --no-seektable \"%s\"",
		ini[L"Tools"][L"flac"], quality, src);

	if (!dest.empty()) { // different destination folder
		wstring destFlacPath(src);
		path::change_extension(src, L".flac");
		cmdLine.append( str::format(L" -o \"%s\\%s\"",
			dest, path::file_from(destFlacPath)) );
	}

	_execute(cmdLine, src, delSrc);
}

void Convert::to_mp3(const file_ini& ini, wstring src, wstring dest, bool delSrc,
	const wstring& quality, bool isVbr)
{
	_validate_dest_folder(dest);

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (path::has_extension(src, {L".flac", L".mp3"})) { // needs intermediary WAV conversion
		if (path::has_extension(src, L".flac")) {
			to_wav(ini, src, dest, delSrc); // send WAV straight to new folder, if any
		} else if (path::has_extension(src, L".mp3")) {
			to_wav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc); // if same destination folder, then delete source (will be replaced)
		}

		if (!dest.empty()) { // different destination folder
			src = str::format(L"%s\\%s", dest, path::file_from(src));
			dest.clear();
		}

		path::change_extension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!path::has_extension(src, L".wav")) {
		throw runtime_error(str::to_ascii(
			str::format(L"Not a FLAC/MP3/WAV: %s\n", src) ));
	}

	wstring cmdLine = str::format(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		ini[L"Tools"][L"lame"], (isVbr ? L"V" : L"b"), quality, src);

	if (!dest.empty()) { // different destination folder
		wstring destMp3Path(src);
		path::change_extension(src, L".mp3");
		cmdLine.append( str::format(L" \"%s\\%s\"",
			dest, path::file_from(destMp3Path)) );
	}

	_execute(cmdLine, src, delSrc);
}

void Convert::_validate_dest_folder(wstring& dest)
{
	if (dest.empty()) {
		return; // same destination of source file, it's OK
	}

	path::trim_backslash(dest);

	if (!file::util::is_dir(dest)) {
		throw runtime_error(str::to_ascii(
			str::format(L"Destination is not a folder:\n%s", dest) ));
	}
}

void Convert::_execute(const wstring& cmdLine, const wstring& src, bool delSrc)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	OutputDebugString( str::format(L"Run %s\n", cmdLine).c_str() );
	if (delSrc) {
		OutputDebugString( str::format(L"Del %s\n", src).c_str() );
	}
#endif

	executable::exec(cmdLine); // run tool
	if (delSrc) file::util::del(src); // delete source file
}