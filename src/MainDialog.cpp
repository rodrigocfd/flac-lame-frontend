
#include "MainDialog.h"
#include "RunninDialog.h"
#include "Convert.h"
using namespace wolf;
using namespace wolf::ctrl;
using std::vector;
using std::wstring;

RUN(MainDialog);
void MainDialog::events() {

this->onMessage(WM_INITDIALOG, [&](WPARAM wp, LPARAM lp)->INT_PTR
{
	// Validate and load INI file.
	_ini.setPath( sys::GetExePath().append(L"\\FlacLameFE.ini") );
	
	wstring err;
	if (!_ini.load(&err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return TRUE;
	}

	// Validate tools.
	if (!Convert::PathsAreValid(_ini, &err)) {
		this->messageBox(L"Fail", err, MB_ICONERROR);
		this->sendMessage(WM_CLOSE, 0, 0); // halt program
		return TRUE;
	}

	// Layout control when resizing.
	_resizer.add({ LST_FILES }, this, Resizer::Do::RESIZE, Resizer::Do::RESIZE)
		.add({ TXT_DEST }, this, Resizer::Do::RESIZE, Resizer::Do::REPOS)
		.add({ LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
			CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
			this, Resizer::Do::NOTHING, Resizer::Do::REPOS)
		.add({ BTN_DEST, BTN_RUN }, this, Resizer::Do::REPOS, Resizer::Do::REPOS)
		.afterResize([&]() {
			_lstFiles.columnFit(0);
		});

	// Main listview initialization.
	( _lstFiles = this->getChild(LST_FILES) )
		.columnAdd(L"File", 300)
		.columnFit(0)
		.iconPush(L"mp3")
		.iconPush(L"flac")
		.iconPush(L"wav"); // icons of the 3 filetypes we use

	// Initializing comboboxes.
	_cmbCbr = this->getChild(CMB_CBR);
	_cmbCbr.itemAdd({ L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps", L"64 kbps", L"80 kbps", L"96 kbps",
		L"112 kbps", L"128 kbps; default", L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps" });
	_cmbCbr.itemSetSelected(8);

	_cmbVbr = this->getChild(CMB_VBR);
	_cmbVbr.itemAdd({ L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)",
		L"4 (~165 kbps); default", L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)",
		L"8 (~85 kbps)", L"9 (~65 kbps)" });
	_cmbVbr.itemSetSelected(4);

	_cmbFlac = this->getChild(CMB_FLAC);
	_cmbFlac.itemAdd({ L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8" });
	_cmbFlac.itemSetSelected(7);

	_cmbNumThreads = this->getChild(CMB_NUMTHREADS);
	_cmbNumThreads.itemAdd({ L"1", L"2", L"4", L"8" });
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
	_radMp3    = this->getChild(RAD_MP3); _radMp3.setCheck(true, Radio::EmulateClick::YES);
	_radMp3Cbr = this->getChild(RAD_CBR);
	_radMp3Vbr = this->getChild(RAD_VBR); _radMp3Vbr.setCheck(true, Radio::EmulateClick::YES);
	_radFlac   = this->getChild(RAD_FLAC);
	_radWav    = this->getChild(RAD_WAV);

	_chkDelSrc = this->getChild(CHK_DELSRC);

	return TRUE;
});

this->onMessage(WM_DROPFILES, [&](WPARAM wp, LPARAM lp)->INT_PTR
{
	for (const wstring& drop : this->getDroppedFiles(reinterpret_cast<HDROP>(wp))) {
		if (file::IsDir(drop)) { // if a directory, add all files inside of it
			for (const wstring f : file::Listing::GetAll(drop, L"*.mp3")) {
				this->_doFileToList(f);
			}
			for (const wstring f : file::Listing::GetAll(drop, L"*.flac")) {
				this->_doFileToList(f);
			}
			for (const wstring f : file::Listing::GetAll(drop, L"*.wav")) {
				this->_doFileToList(f);
			}
		} else {
			this->_doFileToList(drop); // add single file
		}
	}
	this->_doUpdateCounter( _lstFiles.items.count() );
	return TRUE;
});

this->onCommand(IDCANCEL, [&]()->INT_PTR
{
	if (!_lstFiles.items.count() || this->getChild(BTN_RUN).isEnabled()) {
		this->sendMessage(WM_CLOSE, 0, 0); // close on ESC only if not processing
	}
	return TRUE;
});

this->onCommand(BTN_DEST, [&]()->INT_PTR
{
	wstring folder;
	if (this->getFolderChoose(folder)) {
		this->getChild(TXT_DEST).setText(folder.c_str()).setFocus();
	}
	return TRUE;
});

this->onCommand(RAD_MP3, [&]()->INT_PTR
{
	_radMp3Cbr.setEnable( _radMp3.isChecked() );
	_cmbCbr.setEnable( _radMp3.isChecked() && _radMp3Cbr.isChecked() );

	_radMp3Vbr.setEnable( _radMp3.isChecked() );
	_cmbVbr.setEnable( _radMp3.isChecked() && _radMp3Vbr.isChecked() );

	this->getChild(LBL_LEVEL).setEnable( _radFlac.isChecked() );
	_cmbFlac.setEnable( _radFlac.isChecked() );
	return TRUE;
});
this->onCommand(RAD_FLAC, [&]()->INT_PTR { return this->sendCommand(RAD_MP3, 0, 0); });
this->onCommand(RAD_WAV, [&]()->INT_PTR { return this->sendCommand(RAD_MP3, 0, 0); });

this->onCommand(RAD_CBR, [&]()->INT_PTR
{
	_cmbCbr.setEnable( _radMp3Cbr.isChecked() );
	_cmbVbr.setEnable( _radMp3Vbr.isChecked() );
	return TRUE;
});
this->onCommand(RAD_VBR, [&]()->INT_PTR { return this->sendCommand(RAD_CBR, 0, 0); });

this->onCommand(BTN_RUN, [&]()->INT_PTR
{
	// Validate destination folder, if any.
	wstring destFolder = this->getChild(TXT_DEST).getText();
	if (!destFolder.empty()) {
		if (!file::Exists(destFolder)) {
			int q = this->messageBox(L"Create directory",
				str::Sprintf(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				if (!file::CreateDir(destFolder)) {
					this->messageBox(L"Fail",
						str::Sprintf(L"The directory failed to be created:\n%s", destFolder.c_str()),
						MB_ICONERROR);
					return TRUE; // halt
				}
			} else { // user didn't want to create the new dir
				return TRUE; // halt
			}
		} else if (!file::IsDir(destFolder)) {
			this->messageBox(L"Fail",
				str::Sprintf(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return TRUE; // halt
		}
	}

	// Check the existence of each file added to list.
	vector<ListView::Item> allItems = _lstFiles.items.getAll();
	vector<wstring> files = ListView::TextsFromItems(allItems, 0);

	for (const wstring& f : files) { // each filepath
		if (!file::Exists(f)) {
			this->messageBox(L"Fail",
				str::Sprintf(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return TRUE; // halt
		}
	}

	// Retrieve settings.
	bool delSrc = _chkDelSrc.isChecked();
	bool isVbr = _radMp3Vbr.isChecked();
	int numThreads = std::stoi(_cmbNumThreads.itemGetSelectedText());
	
	wstring quality;
	if (_radMp3.isChecked()) {
		Combo& cmbQuality = (isVbr ? _cmbVbr : _cmbCbr);
		quality = cmbQuality.itemGetSelectedText();
		quality.resize(str::Find(str::Sens::YES, quality, L' ')); // first characters of chosen option are the quality setting itself
	} else if (_radFlac.isChecked()) {
		quality = _cmbFlac.itemGetSelectedText(); // text is quality setting itself
	}

	// Which format are we converting to?
	RunninDialog::Target targetType = RunninDialog::Target::NONE;
	
	if (_radMp3.isChecked())       targetType = RunninDialog::Target::MP3;
	else if (_radFlac.isChecked()) targetType = RunninDialog::Target::FLAC;
	else if (_radWav.isChecked())  targetType = RunninDialog::Target::WAV;

	// Finally invoke dialog.
	RunninDialog rd(numThreads, targetType, files, delSrc, isVbr, quality, _ini, destFolder);
	rd.show(this);
	return TRUE;
});

this->onNotify(LST_FILES, LVN_INSERTITEM, [&](NMHDR *h)->INT_PTR { return this->_doUpdateCounter(_lstFiles.items.count()); }); // new item inserted
this->onNotify(LST_FILES, LVN_DELETEITEM, [&](NMHDR *h)->INT_PTR { return this->_doUpdateCounter(_lstFiles.items.count() - 1); }); // item about to be deleted
this->onNotify(LST_FILES, LVN_DELETEALLITEMS, [&](NMHDR *h)->INT_PTR { return this->_doUpdateCounter(0); }); // all items about to be deleted

this->onNotify(LST_FILES, LVN_KEYDOWN, [&](NMHDR *h)->INT_PTR
{
	NMLVKEYDOWN* nkd = reinterpret_cast<NMLVKEYDOWN*>(h);
	if (nkd->wVKey == VK_DELETE) { // Del key
		_lstFiles.items.removeSelected();
		return TRUE;
	}
	return FALSE;
});

}//events


INT_PTR MainDialog::_doUpdateCounter(int newCount)
{
	// Update counter on Run button.
	wstring caption;
	
	if (newCount) caption = str::Sprintf(L"&Run (%d)", newCount);
	else caption = L"&Run";

	this->getChild(BTN_RUN)
		.setText(caption)
		.setEnable(newCount > 0); // Run button enabled if at least 1 file
	return TRUE;
};

void MainDialog::_doFileToList(const wstring& file)
{
	using namespace str;

	int iType = -1;
	if (EndsWith(Sens::NO, file, L".mp3"))       iType = 0;
	else if (EndsWith(Sens::NO, file, L".flac")) iType = 1;
	else if (EndsWith(Sens::NO, file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!_lstFiles.items.exists(file)) {
		_lstFiles.items.add(file, iType); // add only if not present yet
	}
}