
#include "Convert.h"
#include "../winlamb/file.h"
#include "../winlamb/path.h"
#include "../winlamb/str.h"
#include "../winlamb/sys.h"
using namespace wl;
using std::wstring;

bool Convert::paths_are_valid(const file_ini& ini, wstring* pErr)
{
	if (!ini.has_key(L"Tools", L"lame") || !ini.has_key(L"Tools", L"flac")) {
		if (pErr) *pErr = L"INI file doesn't have the right entries.";
		return false;
	}

	// Search for FLAC and LAME tools.
	if (!file::exists( *ini.val(L"Tools", L"lame") )) {
		if (pErr) *pErr = str::format(L"Could not find LAME tool at:\n%s",
			ini.val(L"Tools", L"lame")->c_str());
		return false;
	}
	if (!file::exists( *ini.val(L"Tools", L"flac") )) {
		if (pErr) *pErr = str::format(L"Could not find FLAC tool at:\n%s",
			ini.val(L"Tools", L"lame")->c_str());
		return false;
	}

	// All good.
	if (pErr) pErr->clear();
	return true;
}

bool Convert::to_wav(const file_ini& ini, wstring src, wstring dest, bool delSrc, wstring* pErr)
{
	if (!_check_dest_folder(dest, pErr)) {
		return false;
	}

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	wstring cmdLine;
	if (path::has_extension(src, L".mp3")) {
		cmdLine = str::format(L"\"%s\" --decode \"%s\"",
			ini.val(L"Tools", L"lame")->c_str(), src.c_str());
	} else if (path::has_extension(src, L".flac")) {
		cmdLine = str::format(L"\"%s\" -d \"%s\"",
			ini.val(L"Tools", L"flac")->c_str(), src.c_str());
		if (!dest.empty()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		if (pErr) *pErr = str::format(L"Not a FLAC/MP3: %s\n", src.c_str());
		return false;
	}

	if (!dest.empty()) { // different destination folder
		wstring destWavPath(src);
		path::change_extension(destWavPath, L".wav");
		cmdLine.append( str::format(L" \"%s\\%s\"",
			dest.c_str(), path::file_from(destWavPath).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::to_flac(const file_ini& ini, wstring src, wstring dest, bool delSrc,
	const wstring& quality, wstring* pErr)
{
	if (!_check_dest_folder(dest, pErr)) {
		return false;
	}

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (path::has_extension(src, { L".flac", L".mp3" })) { // needs intermediary WAV conversion
		if (path::has_extension(src, L".mp3")) {
			if (!to_wav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (path::has_extension(src, L".flac")) {
			to_wav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr);
		}

		if (!dest.empty()) { // different destination folder
			src = str::format(L"%s\\%s", dest.c_str(), path::file_from(src).c_str());
			dest.clear();
		}

		path::change_extension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!path::has_extension(src, L".wav")) {
		if (pErr) *pErr = str::format(L"Not a FLAC/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = str::format(L"\"%s\" -%s -V --no-seektable \"%s\"",
		ini.val(L"Tools", L"flac")->c_str(), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destFlacPath(src);
		path::change_extension(src, L".flac");
		cmdLine.append( str::format(L" -o \"%s\\%s\"",
			dest.c_str(), path::file_from(destFlacPath).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::to_mp3(const file_ini& ini, wstring src, wstring dest, bool delSrc,
	const wstring& quality, bool isVbr, wstring *pErr)
{
	if (!_check_dest_folder(dest, pErr)) {
		return false;
	}

	if (path::is_same(path::folder_from(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (path::has_extension(src, { L".flac", L".mp3" })) { // needs intermediary WAV conversion
		if (path::has_extension(src, L".flac")) {
			if (!to_wav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (path::has_extension(src, L".mp3")) {
			if (!to_wav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr))
				return false;
		}

		if (!dest.empty()) { // different destination folder
			src = str::format(L"%s\\%s", dest.c_str(), path::file_from(src).c_str());
			dest.clear();
		}

		path::change_extension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!path::has_extension(src, L".wav")) {
		if (pErr) *pErr = str::format(L"Not a FLAC/MP3/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = str::format(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		ini.val(L"Tools", L"lame")->c_str(), (isVbr ? L"V" : L"b"), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destMp3Path(src);
		path::change_extension(src, L".mp3");
		cmdLine.append( str::format(L" \"%s\\%s\"",
			dest.c_str(), path::file_from(destMp3Path).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::_check_dest_folder(wstring& dest, wstring *pErr)
{
	if (dest.empty()) {
		return true; // same destination of source file, it's OK
	}
	path::trim_backslash(dest);
	if (!file::is_dir(dest)) {
		if (pErr) *pErr = str::format(L"Destination is not a folder:\n%s", dest.c_str());
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::_execute(const wstring& cmdLine, const wstring& src, bool delSrc, wstring* pErr)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	OutputDebugString( str::format(L"Run %s\n", cmdLine.c_str()).c_str() );
	if (delSrc) {
		OutputDebugString( str::format(L"Del %s\n", src.c_str()).c_str() );
	}
#endif

	sys::exec(cmdLine); // run tool
	if (delSrc) {
		if (!file::del(src, pErr)) { // delete source file
			return false;
		}
	}
	if (pErr) pErr->clear();
	return true;
}