
#include "WndMain.h"
#include "WndRunnin.h"
#include "Convert.h"
#include "../res/resource.h"
using namespace wolf;
using std::vector;
using std::wstring;

RUN(WndMain);

WndMain::WndMain()
	: _resizer(this)
{
	this->setup.dialogId = DLG_MAIN;
	this->setup.iconId = ICO_MAIN;

	this->onMessage(WM_CREATE, [this](WPARAM wp, LPARAM lp)->LRESULT
	{
		// Validate and load INI file.
		wstring iniPath = Sys::pathOfExe().append(L"\\FlacLameFE.ini");
		if (!File::exists(iniPath)) {
			Sys::msgBox(this, L"Fail",
				Str::format(L"Fail", L"File not found:\n%s", iniPath.c_str()),
				MB_ICONERROR);
			this->sendMessage(WM_CLOSE, 0, 0); // halt program
			return 0;
		}

		wstring err;
		if (!_ini.load(iniPath, &err)) {
			Sys::msgBox(this, L"Fail",
				Str::format(L"Fail", L"Failed to open:\n%s\n%s", iniPath.c_str(), err.c_str()),
				MB_ICONERROR);
			this->sendMessage(WM_CLOSE, 0, 0); // halt program
			return 0;
		}

		// Validate tools.
		if (!Convert::pathsAreValid(_ini, &err)) {
			Sys::msgBox(this, L"Fail", err, MB_ICONERROR);
			this->sendMessage(WM_CLOSE, 0, 0); // halt program
			return 0;
		}

		// Main listview initialization.
		( _lstFiles = this->getChild(LST_FILES) )
			.columnAdd(L"File", 300)
			.columnFit(0)
			.iconPush(L"mp3")
			.iconPush(L"flac")
			.iconPush(L"wav"); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		_cmbCbr = this->getChild(CMB_CBR);
		_cmbCbr.itemAdd(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
			L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps");
		_cmbCbr.itemSetSelected(8);

		_cmbVbr = this->getChild(CMB_VBR);
		_cmbVbr.itemAdd(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
			L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)"
			L"8 (~85 kbps)|9 (~65 kbps)");
		_cmbVbr.itemSetSelected(4);

		_cmbFlac = this->getChild(CMB_FLAC);
		_cmbFlac.itemAdd(L"1|2|3|4|5|6|7|8");
		_cmbFlac.itemSetSelected(7);

		_cmbNumThreads = this->getChild(CMB_NUMTHREADS);
		_cmbNumThreads.itemAdd(L"1|2|4|8");
		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		switch (si.dwNumberOfProcessors) {
			case 1: _cmbNumThreads.itemSetSelected(0); break;
			case 2: _cmbNumThreads.itemSetSelected(1); break;
			case 4: _cmbNumThreads.itemSetSelected(2); break;
			case 8: _cmbNumThreads.itemSetSelected(3); break;
			default: _cmbNumThreads.itemSetSelected(0);
		}

		// Initializing radio buttons.
		_radMp3    = this->getChild(RAD_MP3); _radMp3.setCheckAndTrigger(true);
		_radMp3Cbr = this->getChild(RAD_CBR);
		_radMp3Vbr = this->getChild(RAD_VBR); _radMp3Vbr.setCheckAndTrigger(true);
		_radFlac   = this->getChild(RAD_FLAC);
		_radWav    = this->getChild(RAD_WAV);

		_chkDelSrc = this->getChild(CHK_DELSRC);

		// Layout control when resizing.
		_resizer.addByObj({ &_lstFiles }, Resizer::Do::RESIZE, Resizer::Do::RESIZE)
			.addById({ TXT_DEST }, Resizer::Do::RESIZE, Resizer::Do::REPOS)
			.addById({ LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
				CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
				Resizer::Do::NOTHING, Resizer::Do::REPOS)
			.addById({ BTN_DEST, BTN_RUN }, Resizer::Do::REPOS, Resizer::Do::REPOS)
			.afterResize([this]()->void {
				_lstFiles.columnFit(0);
			});

		return 0;
	});

	this->onMessage(WM_DROPFILES, [this](WPARAM wp, LPARAM lp)->LRESULT
	{
		for (const wstring& drop : Sys::getDroppedFiles(reinterpret_cast<HDROP>(wp))) {
			if (File::isDir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : File::listDir(drop.c_str(), L"*.mp3")) {
					this->_doFileToList(f);
				}
				for (const wstring& f : File::listDir(drop.c_str(), L"*.flac")) {
					this->_doFileToList(f);
				}
				for (const wstring& f : File::listDir(drop.c_str(), L"*.wav")) {
					this->_doFileToList(f);
				}
			} else {
				this->_doFileToList(drop); // add single file
			}
		}
		this->_doUpdateCounter( _lstFiles.items.count() );
		return 0;
	});

	this->onCommand(IDCANCEL, [this]()->LRESULT
	{
		if (!_lstFiles.items.count() || IsWindowEnabled(this->getChild(BTN_RUN).hWnd())) {
			this->sendMessage(WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return 0;
	});

	this->onCommand(BTN_DEST, [this]()->LRESULT
	{
		wstring folder;
		if (File::showChooseFolder(this, folder)) {
			this->getChild(TXT_DEST).setText(folder.c_str());
			SetFocus(this->getChild(TXT_DEST).hWnd());
		}
		return 0;
	});

	this->onCommand(RAD_MP3, [this]()->LRESULT
	{
		EnableWindow(_radMp3Cbr.hWnd(), _radMp3.isChecked());
		EnableWindow(_cmbCbr.hWnd(), _radMp3.isChecked() && _radMp3Cbr.isChecked());

		EnableWindow(_radMp3Vbr.hWnd(), _radMp3.isChecked());
		EnableWindow(_cmbVbr.hWnd(), _radMp3.isChecked() && _radMp3Vbr.isChecked());

		EnableWindow(this->getChild(LBL_LEVEL).hWnd(), _radFlac.isChecked());
		EnableWindow(_cmbFlac.hWnd(), _radFlac.isChecked());
		return 0;
	});
	this->onCommand(RAD_FLAC, [this]()->LRESULT { return this->sendMessage(WM_COMMAND, MAKEWPARAM(RAD_MP3, 0), 0); });
	this->onCommand(RAD_WAV, [this]()->LRESULT { return this->sendMessage(WM_COMMAND, MAKEWPARAM(RAD_MP3, 0), 0); });

	this->onCommand(RAD_CBR, [this]()->LRESULT
	{
		EnableWindow(_cmbCbr.hWnd(), _radMp3Cbr.isChecked());
		EnableWindow(_cmbVbr.hWnd(), _radMp3Vbr.isChecked());
		return 0;
	});
	this->onCommand(RAD_VBR, [this]()->LRESULT { return this->sendMessage(WM_COMMAND, MAKEWPARAM(RAD_CBR, 0), 0); });

	this->onCommand(BTN_RUN, [this]()->LRESULT
	{
		if (!this->_destFolderIsOk() || !this->_filesExist()) {
			return 0;
		}

		// Retrieve settings.
		bool delSrc = _chkDelSrc.isChecked();
		bool isVbr = _radMp3Vbr.isChecked();
		int numThreads = std::stoi(_cmbNumThreads.itemGetSelectedText());
	
		wstring quality;
		if (_radMp3.isChecked()) {
			ComboBox& cmbQuality = (isVbr ? _cmbVbr : _cmbCbr);
			quality = cmbQuality.itemGetSelectedText();
			quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
		} else if (_radFlac.isChecked()) {
			quality = _cmbFlac.itemGetSelectedText(); // text is quality setting itself
		}

		// Which format are we converting to?
		WndRunnin::Target targetType = WndRunnin::Target::NONE;
	
		if (_radMp3.isChecked())       targetType = WndRunnin::Target::MP3;
		else if (_radFlac.isChecked()) targetType = WndRunnin::Target::FLAC;
		else if (_radWav.isChecked())  targetType = WndRunnin::Target::WAV;

		// Finally invoke dialog.
		WndRunnin rd(this, numThreads, targetType,
			ListView::getAllText(_lstFiles.items.getAll(), 0),
			delSrc, isVbr, quality, _ini,
			this->getChild(TXT_DEST).getText());
		rd.show(this);
		return 0;
	});

	this->onNotify(LST_FILES, LVN_INSERTITEM, [this](NMHDR& nm)->LRESULT { return this->_doUpdateCounter(_lstFiles.items.count()); }); // new item inserted
	this->onNotify(LST_FILES, LVN_DELETEITEM, [this](NMHDR& nm)->LRESULT { return this->_doUpdateCounter(_lstFiles.items.count() - 1); }); // item about to be deleted
	this->onNotify(LST_FILES, LVN_DELETEALLITEMS, [this](NMHDR& nm)->LRESULT { return this->_doUpdateCounter(0); }); // all items about to be deleted

	this->onNotify(LST_FILES, LVN_KEYDOWN, [this](NMHDR& nm)->LRESULT
	{
		NMLVKEYDOWN& nkd = reinterpret_cast<NMLVKEYDOWN&>(nm);
		if (nkd.wVKey == VK_DELETE) { // Del key
			_lstFiles.items.removeSelected();
		}
		return 0;
	});
}

bool WndMain::_destFolderIsOk()
{
	wstring destFolder = this->getChild(TXT_DEST).getText();
	if (!destFolder.empty()) {
		if (!File::exists(destFolder)) {
			int q = Sys::msgBox(this, L"Create directory",
				Str::format(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				wstring err;
				if (!File::createDir(destFolder, &err)) {
					Sys::msgBox(this, L"Fail",
						Str::format(L"The directory failed to be created:\n%s\n%s", destFolder.c_str(), err.c_str()),
						MB_ICONERROR);
					return false; // halt
				}
			} else { // user didn't want to create the new dir
				return false; // halt
			}
		} else if (!File::isDir(destFolder)) {
			Sys::msgBox(this, L"Fail",
				Str::format(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

bool WndMain::_filesExist()
{
	vector<ListView::Item> allItems = _lstFiles.items.getAll();
	vector<wstring> files = ListView::getAllText(allItems, 0);

	for (const wstring& f : files) { // each filepath
		if (!File::exists(f)) {
			Sys::msgBox(this, L"Fail",
				Str::format(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

LRESULT WndMain::_doUpdateCounter(int newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		Str::format(L"&Run (%d)", newCount) : L"&Run";

	this->getChild(BTN_RUN).setText(caption);
	EnableWindow(this->getChild(BTN_RUN).hWnd(), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void WndMain::_doFileToList(const wstring& file)
{
	int iType = -1;
	if (Str::endsWithI(file, L".mp3"))       iType = 0;
	else if (Str::endsWithI(file, L".flac")) iType = 1;
	else if (Str::endsWithI(file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!_lstFiles.items.exists(file)) {
		_lstFiles.items.add(file, iType); // add only if not present yet
	}
}