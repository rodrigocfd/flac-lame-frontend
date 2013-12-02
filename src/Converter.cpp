
#include "../toolow/File.h"
#include "../toolow/util.h"
#include "Converter.h"

Converter::Converter(HWND hParent, File::Ini *pIni, const wchar_t *src, bool delSrc, const wchar_t *quality, bool isVbr, const wchar_t *dest)
	: _hParent(hParent), _pIni(pIni), _srcPath(src), _delSrc(delSrc), _isVbr(isVbr)
{
	if(dest) {
		_destPath = dest;
		if(_destPath.endsWith(L'\\'))
			_destPath[_destPath.len() - 1] = L'\0'; // store path with no final backslash
	}

	if(quality)
		lstrcpy(_quality, quality);
}

bool Converter::PathsAreValid(File::Ini *pIni, String *pErr)
{
	// Search for FLAC and LAME tools.
	if(!File::Exists( pIni->sections[L"Tools"][L"lame"].str() )) {
		if(pErr) pErr->fmt(L"Could not find LAME tool at:\n%s", pIni->sections[L"Tools"][L"lame"].str());
		return false;
	}
	if(!File::Exists( pIni->sections[L"Tools"][L"flac"].str() )) {
		if(pErr) pErr->fmt(L"Could not find FLAC tool at:\n%s", pIni->sections[L"Tools"][L"lame"].str());
		return false;
	}

	// All good.
	if(pErr) (*pErr) = L"";
	return true;
}

void Converter::_doExec(String *cmdLine)
{
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	debug(L"Run %s\n", cmdLine->str());
	if(_delSrc) debug(L"Del %s\n", _srcPath.str());
#endif

	exec(cmdLine->str()); // run tool
	if(_delSrc) DeleteFile(_srcPath.str()); // delete source file
}

void ConverterMp3::onRun()
{
	if(_srcPath.endsWith(L".flac", String::INSENS) || _srcPath.endsWith(L".mp3", String::INSENS)) { // needs intermediary WAV file
		ConverterWav *towav = _srcPath.endsWith(L".flac", String::INSENS) ?
			new ConverterWav(0, _pIni, _srcPath.str(), _delSrc, _destPath.str()) : // send WAV straight to new folder, if any
			new ConverterWav(0, _pIni, _srcPath.str(),
				_destPath.isEmpty() ? true : _delSrc, // if same destination folder, then delete source (will be replaced)
				_destPath.str()); // send WAV straight to new folder, if any

		towav->runSync(); // will halt until complete

		if(!_destPath.isEmpty()) {
			String name = &_srcPath[_srcPath.findr(L'\\') + 1]; // file name only
			_srcPath.fmt(L"%s\\%s", _destPath.str(), name.str()); // our target WAV is now on destination folder
			_destPath = L"";
		}

		_srcPath[_srcPath.findr(L'.')] = L'\0';
		_srcPath.append(L".wav"); // our source is now a WAV
		_delSrc = true; // delete the WAV at end
	} else if(!_srcPath.endsWith(L".wav", String::INSENS)) {
		debug(L"Not a FLAC/MP3/WAV: %s\n", _srcPath.str());
		return;
	}

	String cmdLine;
	cmdLine.fmt(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		_pIni->sections[L"Tools"][L"lame"].str(), (_isVbr ? L"V" : L"b"), _quality, _srcPath.str());

	if(!_destPath.isEmpty()) { // different destination folder
		cmdLine.appendfmt(L" \"%s\\%s\"", _destPath.str(), &_srcPath[_srcPath.findr(L'\\') + 1]);
		cmdLine[cmdLine.findr(L'.')] = L'\0';
		cmdLine.append(L".mp3\""); // MP3 extension on destination file
	}

	this->_doExec(&cmdLine);

	// Notify parent on conclusion.
	// Notice that intermediary WAV conversions initialize HWND with zero, thus not notificating.
	if(_hParent)
		SendMessage(_hParent, WM_FILEDONE, 0, 0);
}

void ConverterFlac::onRun()
{
	if(_srcPath.endsWith(L".flac", String::INSENS) || _srcPath.endsWith(L".mp3", String::INSENS)) { // needs intermediary WAV file
		ConverterWav *towav = _srcPath.endsWith(L".mp3", String::INSENS) ?
			new ConverterWav(0, _pIni, _srcPath.str(), _delSrc, _destPath.str()) : // send WAV straight to new folder, if any
			new ConverterWav(0, _pIni, _srcPath.str(),
				_destPath.isEmpty() ? true : _delSrc, // if same destination folder, then delete source (will be replaced)
				_destPath.str()); // send WAV straight to new folder, if any

		towav->runSync(); // will halt until complete

		if(!_destPath.isEmpty()) {
			String name = &_srcPath[_srcPath.findr(L'\\') + 1]; // file name only
			_srcPath.fmt(L"%s\\%s", _destPath.str(), name.str()); // our target WAV is now on destination folder
			_destPath = L"";
		}

		_srcPath[_srcPath.findr(L'.')] = L'\0';
		_srcPath.append(L".wav"); // our source is now a WAV
		_delSrc = true; // delete the WAV at end
	} else if(!_srcPath.endsWith(L".wav", String::INSENS)) {
		debug(L"Not a FLAC/WAV: %s\n", _srcPath.str());
		return;
	}

	String cmdLine;
	cmdLine.fmt(L"\"%s\" -%s -V --no-seektable \"%s\"",
		_pIni->sections[L"Tools"][L"flac"].str(), _quality, _srcPath.str());
	
	if(!_destPath.isEmpty()) { // different destination folder
		cmdLine.appendfmt(L" -o \"%s\\%s\"", _destPath.str(), &_srcPath[_srcPath.findr(L'\\') + 1]);
		cmdLine[cmdLine.findr(L'.')] = L'\0';
		cmdLine.append(L".flac\""); // FLAC extension on destination file
	}
	
	this->_doExec(&cmdLine);

	// Notify parent on conclusion.
	// Notice that intermediary WAV conversions initialize HWND with zero, thus not notificating.
	if(_hParent)
		SendMessage(_hParent, WM_FILEDONE, 0, 0);
}

void ConverterWav::onRun()
{
	String cmdLine;

	if(_srcPath.endsWith(L".mp3", String::INSENS)) {
		cmdLine.fmt(L"\"%s\" --decode \"%s\"", _pIni->sections[L"Tools"][L"lame"].str(), _srcPath.str());
	} else if(_srcPath.endsWith(L".flac", String::INSENS)) {
		cmdLine.fmt(L"\"%s\" -d \"%s\"", _pIni->sections[L"Tools"][L"flac"].str(), _srcPath.str());
		if(!_destPath.isEmpty())
			cmdLine.append(L" -o"); // different destination folder requires flag
	} else {
		debug(L"Not a FLAC/MP3: %s\n", _srcPath.str());
		return;
	}

	if(!_destPath.isEmpty()) { // different destination folder
		cmdLine.appendfmt(L" \"%s\\%s\"", _destPath.str(), &_srcPath[_srcPath.findr(L'\\') + 1]);
		cmdLine[cmdLine.findr(L'.')] = L'\0';
		cmdLine.append(L".wav\""); // WAV extension on destination file
	}

	this->_doExec(&cmdLine);

	// Notify parent on conclusion.
	// Notice that intermediary WAV conversions initialize HWND with zero, thus not notificating.
	if(_hParent)
		SendMessage(_hParent, WM_FILEDONE, 0, 0);
}