
#include "Convert.h"
#include "../winutil/File.h"
#include "../winutil/Path.h"
#include "../winutil/Str.h"
#include "../winutil/Sys.h"
using std::wstring;

bool Convert::pathsAreValid(const FileIni& ini, wstring *pErr)
{
	if (!ini.hasKey(L"Tools", L"lame") || !ini.hasKey(L"Tools", L"flac")) {
		if (pErr) *pErr = L"INI file doesn't have the right entries.";
		return false;
	}

	// Search for FLAC and LAME tools.
	if (!File::exists( ini.val(L"Tools", L"lame") )) {
		if (pErr) *pErr = Str::format(L"Could not find LAME tool at:\n%s", ini.val(L"Tools", L"lame").c_str());
		return false;
	}
	if (!File::exists( ini.val(L"Tools", L"flac") )) {
		if (pErr) *pErr = Str::format(L"Could not find FLAC tool at:\n%s", ini.val(L"Tools", L"lame").c_str());
		return false;
	}

	// All good.
	if (pErr) pErr->clear();
	return true;
}

bool Convert::toWav(const FileIni& ini, wstring src, wstring dest, bool delSrc, wstring *pErr)
{
	if (!_checkDestFolder(dest, pErr)) {
		return false;
	}

	if (Str::eqI(Path::folderFrom(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	wstring cmdLine;
	if (Str::endsWithI(src, L".mp3")) {
		cmdLine = Str::format(L"\"%s\" --decode \"%s\"",
			ini.val(L"Tools", L"lame").c_str(), src.c_str());
	} else if (Str::endsWithI(src, L".flac")) {
		cmdLine = Str::format(L"\"%s\" -d \"%s\"",
			ini.val(L"Tools", L"flac").c_str(), src.c_str());
		if (!dest.empty()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		if (pErr) *pErr = Str::format(L"Not a FLAC/MP3: %s\n", src.c_str());
		return false;
	}

	if (!dest.empty()) { // different destination folder
		wstring destWavPath(src);
		Path::changeExtension(destWavPath, L".wav");
		cmdLine.append( Str::format(L" \"%s\\%s\"",
			dest.c_str(), Path::fileFrom(destWavPath).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}
	
bool Convert::toFlac(const FileIni& ini, wstring src, wstring dest, bool delSrc, const wstring& quality, wstring *pErr)
{
	if (!_checkDestFolder(dest, pErr)) {
		return false;
	}

	if (Str::eqI(Path::folderFrom(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (Path::hasExtension(src, { L".flac", L".mp3" })) { // needs intermediary WAV conversion
		if (Str::endsWithI(src, L".mp3")) {
			if (!toWav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (Str::endsWithI(src, L".flac")) {
			toWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr);
		}

		if (!dest.empty()) { // different destination folder
			src = Str::format(L"%s\\%s", dest.c_str(), Path::fileFrom(src).c_str());
			dest.clear();
		}

		Path::changeExtension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!Str::endsWithI(src, L".wav")) {
		if (pErr) *pErr = Str::format(L"Not a FLAC/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = Str::format(L"\"%s\" -%s -V --no-seektable \"%s\"",
		ini.val(L"Tools", L"flac").c_str(), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destFlacPath(src);
		Path::changeExtension(src, L".flac");
		cmdLine.append( Str::format(L" -o \"%s\\%s\"",
			dest.c_str(), Path::fileFrom(destFlacPath).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}
	
bool Convert::toMp3(const FileIni& ini, wstring src, wstring dest, bool delSrc, const wstring& quality, bool isVbr, wstring *pErr)
{
	if (!_checkDestFolder(dest, pErr)) {
		return false;
	}

	if (Str::eqI(Path::folderFrom(src), dest)) { // destination folder is same of origin
		dest.clear();
	}

	if (Path::hasExtension(src, { L".flac", L".mp3" })) { // needs intermediary WAV conversion
		if (Str::endsWithI(src, L".flac")) {
			if (!toWav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (Str::endsWithI(src, L".mp3")) {
			if (!toWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr))
				return false;
		}

		if (!dest.empty()) { // different destination folder
			src = Str::format(L"%s\\%s", dest.c_str(), Path::fileFrom(src).c_str());
			dest.clear();
		}

		Path::changeExtension(src, L".wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!Str::endsWithI(src, L".wav")) {
		if (pErr) *pErr = Str::format(L"Not a FLAC/MP3/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = Str::format(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		ini.val(L"Tools", L"lame").c_str(), (isVbr ? L"V" : L"b"), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destMp3Path(src);
		Path::changeExtension(src, L".mp3");
		cmdLine.append( Str::format(L" \"%s\\%s\"",
			dest.c_str(), Path::fileFrom(destMp3Path).c_str()) );
	}

	if (!_execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::_checkDestFolder(wstring& dest, wstring *pErr)
{
	if (dest.empty()) {
		return true; // same destination of source file, it's OK
	}
	Path::trimBackslash(dest);
	if (!File::isDir(dest)) {
		if (pErr) *pErr = Str::format(L"Destination is not a folder:\n%s", dest.c_str());
		return false;
	}
	if (pErr) pErr->clear();
	return true;
}

bool Convert::_execute(const wstring& cmdLine, const wstring& src, bool delSrc, wstring *pErr)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	OutputDebugString( Str::format(L"Run %s\n", cmdLine.c_str()).c_str() );
	if (delSrc) {
		OutputDebugString( Str::format(L"Del %s\n", src.c_str()).c_str() );
	}
#endif

	Sys::exec(cmdLine); // run tool
	if (delSrc) {
		if (!File::del(src, pErr)) { // delete source file
			return false;
		}
	}
	if (pErr) pErr->clear();
	return true;
}