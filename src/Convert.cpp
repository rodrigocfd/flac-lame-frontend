
#include "Convert.h"

bool Convert::PathsAreValid(const File::Ini& ini, String *pErr)
{
	if ( !ini.sections.exists(L"Tools") ||
		!ini.sections[L"Tools"].exists(L"lame") ||
		!ini.sections[L"Tools"].exists(L"flac") )
	{
		if (pErr) *pErr = L"INI file doesn't have the right entries.";
		return false;
	}

	// Search for FLAC and LAME tools.
	if (!File::Exists(ini.sections[L"Tools"][L"lame"])) {
		if (pErr) *pErr = String::Fmt(L"Could not find LAME tool at:\n%s", ini.sections[L"Tools"][L"lame"].str());
		return false;
	}
	if (!File::Exists(ini.sections[L"Tools"][L"flac"])) {
		if (pErr) *pErr = String::Fmt(L"Could not find FLAC tool at:\n%s", ini.sections[L"Tools"][L"lame"].str());
		return false;
	}

	// All good.
	if (pErr) (*pErr) = L"";
	return true;
}

bool Convert::ToWav(const File::Ini& ini, String src, String dest, bool delSrc, String *pErr)
{
	if (!_CheckDestFolder(dest, pErr))
		return false;

	if (File::Path::GetPath(src).equalsCI(dest)) // destination folder is same of origin
		dest = L"";

	String cmdLine;

	if (src.endsWithCI(L".mp3")) {
		cmdLine = String::Fmt(L"\"%s\" --decode \"%s\"", ini.sections[L"Tools"][L"lame"].str(), src.str());
	} else if (src.endsWithCI(L".flac")) {
		cmdLine = String::Fmt(L"\"%s\" -d \"%s\"", ini.sections[L"Tools"][L"flac"].str(), src.str());
		if (!dest.isEmpty())
			cmdLine.append(L" -o"); // different destination folder requires flag
	} else {
		if (pErr) *pErr = String::Fmt(L"Not a FLAC/MP3: %s\n", src.str());
		return false;
	}

	if (!dest.isEmpty()) { // different destination folder
		String destWavPath(src);
		File::Path::ChangeExtension(destWavPath, L"wav");
		cmdLine.append( String::Fmt(L" \"%s\\%s\"", dest.str(), File::Path::GetFilename(destWavPath)) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr))
		return false;

	if (pErr) *pErr = L"";
	return true;
}
	
bool Convert::ToFlac(const File::Ini& ini, String src, String dest, bool delSrc, const String& quality, String *pErr)
{
	if (!_CheckDestFolder(dest, pErr))
		return false;

	if (File::Path::GetPath(src).equalsCI(dest)) // destination folder is same of origin
		dest = L"";

	if (src.endsWithCI(L".flac") || src.endsWithCI(L".mp3")) { // needs intermediary WAV conversion
		if (src.endsWithCI(L".mp3")) {
			if (!ToWav(ini, src, dest, delSrc, pErr)) // send WAV straight to new folder, if any
				return false;
		} else if (src.endsWithCI(L".flac")) {
			ToWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.isEmpty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr);
		}

		if (!dest.isEmpty()) { // different destination folder
			src = String::Fmt(L"%s\\%s", dest.str(), File::Path::GetFilename(src));
			dest = L"";
		}

		File::Path::ChangeExtension(src, L"wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!src.endsWithCI(L".wav")) {
		if (pErr) *pErr = String::Fmt(L"Not a FLAC/WAV: %s\n", src.str());
		return false;
	}

	String cmdLine = String::Fmt(L"\"%s\" -%s -V --no-seektable \"%s\"",
		ini.sections[L"Tools"][L"flac"].str(), quality.str(), src.str());

	if (!dest.isEmpty()) { // different destination folder
		String destFlacPath(src);
		File::Path::ChangeExtension(src, L"flac");
		cmdLine.append( String::Fmt(L" -o \"%s\\%s\"", dest.str(), File::Path::GetFilename(destFlacPath)) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr))
		return false;

	if (pErr) *pErr = L"";
	return true;
}
	
bool Convert::ToMp3(const File::Ini& ini, String src, String dest, bool delSrc, const String& quality, bool isVbr, String *pErr)
{
	if (!_CheckDestFolder(dest, pErr))
		return false;

	if (File::Path::GetPath(src).equalsCI(dest)) // destination folder is same of origin
		dest = L"";

	if (src.endsWithCI(L".flac") || src.endsWithCI(L".mp3")) { // needs intermediary WAV conversion
		if (src.endsWithCI(L".flac")) {
			if (!ToWav(ini, src, dest, delSrc, pErr)) // send WAV straight to new folder, if any
				return false;
		} else if (src.endsWithCI(L".mp3")) {
			if (!ToWav(ini, src, dest, // send WAV straight to new folder, if any
				dest.isEmpty() ? true : delSrc, // if same destination folder, then delete source (will be replaced)
				pErr))
				return false;
		}

		if (!dest.isEmpty()) { // different destination folder
			src = String::Fmt(L"%s\\%s", dest.str(), File::Path::GetFilename(src));
			dest = L"";
		}

		File::Path::ChangeExtension(src, L"wav"); // our source is now a WAV
		delSrc = true; // delete the WAV at end
	} else if (!src.endsWithCI(L".wav")) {
		if (pErr) *pErr = String::Fmt(L"Not a FLAC/MP3/WAV: %s\n", src.str());
		return false;
	}

	String cmdLine = String::Fmt(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		ini.sections[L"Tools"][L"lame"].str(), (isVbr ? L"V" : L"b"), quality.str(), src.str());

	if (!dest.isEmpty()) { // different destination folder
		String destMp3Path(src);
		File::Path::ChangeExtension(src, L"mp3");
		cmdLine.append( String::Fmt(L" \"%s\\%s\"", dest.str(), File::Path::GetFilename(destMp3Path)) );
	}

	if (!_Execute(cmdLine, src, delSrc, pErr))
		return false;

	if (pErr) *pErr = L"";
	return true;
}

bool Convert::_CheckDestFolder(String& dest, String *pErr)
{
	if (dest.isEmpty())
		return true; // same destination of source file, it's OK

	if (!File::IsDir(dest)) {
		if (pErr) *pErr = String::Fmt(L"Destination is not a folder:\n%s", dest.str());
		return false;
	}

	File::Path::TrimBackslash(dest);
	if (pErr) *pErr = L"";
	return true;
}

bool Convert::_Execute(const String& cmdLine, const String& src, bool delSrc, String *pErr)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	dbg(L"Run %s\n", cmdLine.str());
	if (delSrc) dbg(L"Del %s\n", src.str());
#endif

	System::Exec(cmdLine.str()); // run tool
	if (delSrc) {
		if (!File::Delete(src, pErr)) // delete source file
			return false;
	}

	if (pErr) *pErr = L"";
	return true;
}