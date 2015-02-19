
#include "Convert.h"

bool Convert::PathsAreValid(const File::Ini& ini, wstring *pErr)
{
	if ( ini.sections.find(L"Tools") == ini.sections.end() ||
		ini.sections.at(L"Tools").find(L"lame") == ini.sections.at(L"Tools").end() ||
		ini.sections.at(L"Tools").find(L"flac") == ini.sections.at(L"Tools").end() )
	{
		if (pErr) *pErr = L"INI file doesn't have the right entries.";
		return false;
	}

	// Search for FLAC and LAME tools.
	if (!File::Exists(ini.sections.at(L"Tools").at(L"lame"))) {
		if (pErr) *pErr = Sprintf(L"Could not find LAME tool at:\n%s", ini.sections.at(L"Tools").at(L"lame").c_str());
		return false;
	}
	if (!File::Exists(ini.sections.at(L"Tools").at(L"flac"))) {
		if (pErr) *pErr = Sprintf(L"Could not find FLAC tool at:\n%s", ini.sections.at(L"Tools").at(L"lame").c_str());
		return false;
	}

	// All good.
	if (pErr) *pErr = L"";
	return true;
}

bool Convert::ToWav(const File::Ini& ini, wstring src, wstring dest, bool delSrc, wstring *pErr)
{
	if (!_CheckDestFolder(dest, pErr)) {
		return false;
	}

	if (StrEqi(File::Path::GetPath(src), dest)) { // destination folder is same of origin
		dest = L"";
	}

	wstring cmdLine;
	if (EndsWithi(src, L".mp3")) {
		cmdLine = Sprintf(L"\"%s\" --decode \"%s\"",
			ini.sections.at(L"Tools").at(L"lame").c_str(), src.c_str());
	} else if (EndsWithi(src, L".flac")) {
		cmdLine = Sprintf(L"\"%s\" -d \"%s\"",
			ini.sections.at(L"Tools").at(L"flac").c_str(), src.c_str());
		if (!dest.empty()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		if (pErr) *pErr = Sprintf(L"Not a FLAC/MP3: %s\n", src.c_str());
		return false;
	}

	if (!dest.empty()) { // different destination folder
		wstring destWavPath(src);
		File::Path::ChangeExtension(destWavPath, L"wav");
		cmdLine.append( Sprintf(L" \"%s\\%s\"", dest.c_str(), File::Path::GetFilename(destWavPath).c_str()) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) *pErr = L"";
	return true;
}
	
bool Convert::ToFlac(const File::Ini& ini, wstring src, wstring dest, bool delSrc, const wstring& quality, wstring *pErr)
{
	if (!_CheckDestFolder(dest, pErr)) {
		return false;
	}

	if (StrEqi(File::Path::GetPath(src), dest)) { // destination folder is same of origin
		dest = L"";
	}

	if (EndsWithi(src, L".flac") || EndsWithi(src, L".mp3")) { // needs intermediary WAV conversion
		if (EndsWithi(src, L".mp3")) {
			if (!ToWav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (EndsWithi(src, L".flac")) {
			ToWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr);
		}

		if (!dest.empty()) { // different destination folder
			src = Sprintf(L"%s\\%s", dest.c_str(), File::Path::GetFilename(src).c_str());
			dest = L"";
		}

		File::Path::ChangeExtension(src, L"wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!EndsWithi(src, L".wav")) {
		if (pErr) *pErr = Sprintf(L"Not a FLAC/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = Sprintf(L"\"%s\" -%s -V --no-seektable \"%s\"",
		ini.sections.at(L"Tools").at(L"flac").c_str(), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destFlacPath(src);
		File::Path::ChangeExtension(src, L"flac");
		cmdLine.append( Sprintf(L" -o \"%s\\%s\"", dest.c_str(), File::Path::GetFilename(destFlacPath).c_str()) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) *pErr = L"";
	return true;
}
	
bool Convert::ToMp3(const File::Ini& ini, wstring src, wstring dest, bool delSrc, const wstring& quality, bool isVbr, wstring *pErr)
{
	if (!_CheckDestFolder(dest, pErr)) {
		return false;
	}

	if (StrEqi(File::Path::GetPath(src), dest)) { // destination folder is same of origin
		dest = L"";
	}

	if (EndsWithi(src, L".flac") || EndsWithi(src, L".mp3")) { // needs intermediary WAV conversion
		if (EndsWithi(src, L".flac")) {
			if (!ToWav(ini, src, dest, delSrc, pErr)) { // send WAV straight to new folder, if any
				return false;
			}
		} else if (EndsWithi(src, L".mp3")) {
			if (!ToWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.empty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr))
				return false;
		}

		if (!dest.empty()) { // different destination folder
			src = Sprintf(L"%s\\%s", dest.c_str(), File::Path::GetFilename(src).c_str());
			dest = L"";
		}

		File::Path::ChangeExtension(src, L"wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!EndsWithi(src, L".wav")) {
		if (pErr) *pErr = Sprintf(L"Not a FLAC/MP3/WAV: %s\n", src.c_str());
		return false;
	}

	wstring cmdLine = Sprintf(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		ini.sections.at(L"Tools").at(L"lame").c_str(), (isVbr ? L"V" : L"b"), quality.c_str(), src.c_str());

	if (!dest.empty()) { // different destination folder
		wstring destMp3Path(src);
		File::Path::ChangeExtension(src, L"mp3");
		cmdLine.append( Sprintf(L" \"%s\\%s\"", dest.c_str(), File::Path::GetFilename(destMp3Path).c_str()) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr)) {
		return false;
	}
	if (pErr) *pErr = L"";
	return true;
}

bool Convert::_CheckDestFolder(wstring& dest, wstring *pErr)
{
	if (dest.empty()) {
		return true; // same destination of source file, it's OK
	}
	if (!File::IsDir(dest)) {
		if (pErr) *pErr = Sprintf(L"Destination is not a folder:\n%s", dest.c_str());
		return false;
	}

	File::Path::TrimBackslash(dest);
	if (pErr) *pErr = L"";
	return true;
}

bool Convert::_Execute(const wstring& cmdLine, const wstring& src, bool delSrc, wstring *pErr)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	OutputDebugString(Sprintf(L"Run %s\n", cmdLine.c_str()).c_str());
	if (delSrc) {
		OutputDebugString(Sprintf(L"Del %s\n", src.c_str()).c_str());
	}
#endif

	System::Exec(cmdLine); // run tool
	if (delSrc) {
		if (!File::Delete(src, pErr)) { // delete source file
			return false;
		}
	}
	if (pErr) *pErr = L"";
	return true;
}