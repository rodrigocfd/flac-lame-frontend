
#include "DlgMain.h"
#include "DlgRunnin.h"
#include "Convert.h"
#include "../winutil/File.h"
#include "../winutil/Menu.h"
#include "../winutil/Str.h"
#include "../winutil/Sys.h"
#include "../res/resource.h"
using std::vector;
using std::wstring;

RUN(DlgMain);

DlgMain::DlgMain()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;

	on_message(WM_INITDIALOG, [this](WPARAM wp, LPARAM lp)->INT_PTR
	{
		// Validate and load INI file.
		_ini.path = Sys::pathOfExe().append(L"\\FlacLameFE.ini");
		if (!File::exists(_ini.path)) {
			Sys::msgBox(hwnd(), L"Fail",
				Str::format(L"File not found:\n%s", _ini.path.c_str()),
				MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		wstring err;
		if (!_ini.loadFromFile(&err)) {
			Sys::msgBox(hwnd(), L"Fail",
				Str::format(L"Failed to load:\n%s\n%s", _ini.path.c_str(), err.c_str()),
				MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		// Validate tools.
		if (!Convert::pathsAreValid(_ini, &err)) {
			Sys::msgBox(hwnd(), L"Fail", err, MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		_taskBar << hwnd();
		_txtDest = GetDlgItem(hwnd(), TXT_DEST);

		// Main listview initialization.
		(_lstFiles = GetDlgItem(hwnd(), LST_FILES))
			.setContextMenu(MEN_MAIN)
			.columnAdd(L"File", 300)
			.columnFit(0)
			.iconPush(L"mp3")
			.iconPush(L"flac")
			.iconPush(L"wav"); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		_cmbCbr = GetDlgItem(hwnd(), CMB_CBR);
		_cmbCbr.itemAdd(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
			L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps");
		_cmbCbr.itemSetSelected(8);

		_cmbVbr = GetDlgItem(hwnd(), CMB_VBR);
		_cmbVbr.itemAdd(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
			L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)"
			L"8 (~85 kbps)|9 (~65 kbps)");
		_cmbVbr.itemSetSelected(4);

		_cmbFlac = GetDlgItem(hwnd(), CMB_FLAC);
		_cmbFlac.itemAdd(L"1|2|3|4|5|6|7|8");
		_cmbFlac.itemSetSelected(7);

		_cmbNumThreads = GetDlgItem(hwnd(), CMB_NUMTHREADS);
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
		_radMp3    = GetDlgItem(hwnd(), RAD_MP3); _radMp3.setCheckAndTrigger(true);
		_radMp3Cbr = GetDlgItem(hwnd(), RAD_CBR);
		_radMp3Vbr = GetDlgItem(hwnd(), RAD_VBR); _radMp3Vbr.setCheckAndTrigger(true);
		_radFlac   = GetDlgItem(hwnd(), RAD_FLAC);
		_radWav    = GetDlgItem(hwnd(), RAD_WAV);

		_chkDelSrc = GetDlgItem(hwnd(), CHK_DELSRC);

		// Layout control when resizing.
		_resizer.add(hwnd(), LST_FILES, Resizer::Do::RESIZE, Resizer::Do::RESIZE)
			.add(hwnd(), TXT_DEST, Resizer::Do::RESIZE, Resizer::Do::REPOS)
			.add(hwnd(), { LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
				CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
				Resizer::Do::NOTHING, Resizer::Do::REPOS)
			.add(hwnd(), { BTN_DEST, BTN_RUN }, Resizer::Do::REPOS, Resizer::Do::REPOS);

		return TRUE;
	});

	on_message(WM_SIZE, [this](WPARAM wp, LPARAM lp)->INT_PTR
	{
		_resizer.arrange(wp, lp);
		_lstFiles.columnFit(0);
		return TRUE;
	});

	on_message(WM_INITMENUPOPUP, [this](WPARAM wp, LPARAM lp)->INT_PTR
	{
		Menu menu(reinterpret_cast<HMENU>(wp));
		if (menu.getCommandId(0) == MNU_ADDFILES) {
			menu.enableItem(MNU_REMSELECTED, _lstFiles.items.countSelected() > 0);
		}
		return TRUE;
	});

	on_message(WM_DROPFILES, [this](WPARAM wp, LPARAM lp)->INT_PTR
	{
		for (const wstring& drop : Sys::getDroppedFiles(reinterpret_cast<HDROP>(wp))) {
			if (File::isDir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : File::listDir(drop.c_str(), L"*.mp3")) {
					_doFileToList(f);
				}
				for (const wstring& f : File::listDir(drop.c_str(), L"*.flac")) {
					_doFileToList(f);
				}
				for (const wstring& f : File::listDir(drop.c_str(), L"*.wav")) {
					_doFileToList(f);
				}
			} else {
				_doFileToList(drop); // add single file
			}
		}
		_doUpdateCounter( _lstFiles.items.count() );
		return TRUE;
	});

	on_command(MNU_ABOUT, [this]()->INT_PTR
	{
		Sys::msgBox(hwnd(), L"About",
			L"FLAC/LAME graphical front-end.", MB_ICONINFORMATION);
		return TRUE;
	});

	on_command(MNU_ADDFILES, [this]()->INT_PTR
	{
		vector<wstring> files;
		if (File::showOpen(hwnd(),
			L"Supported audio files (*.mp3, *.flac, *.wav)|*.mp3;*.flac;*.wav|"
			L"MP3 audio files (*.mp3)|*.mp3|"
			L"FLAC audio files (*.flac)|*.flac|"
			L"WAV audio files (*.wav)|*.wav",
			files))
		{
			for (const wstring& file : files) {
				_doFileToList(file);
			}
			_doUpdateCounter( _lstFiles.items.count() );
		}
		return TRUE;
	});

	on_command(MNU_REMSELECTED, [this]()->INT_PTR
	{
		_lstFiles.items.removeSelected();
		_doUpdateCounter( _lstFiles.items.count() );
		return TRUE;
	});

	on_command(IDCANCEL, [this]()->INT_PTR
	{
		if (!_lstFiles.items.count() || IsWindowEnabled(GetDlgItem(hwnd(), BTN_RUN))) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return TRUE;
	});

	on_command(BTN_DEST, [this]()->INT_PTR
	{
		wstring folder;
		if (File::showChooseFolder(hwnd(), folder)) {
			_txtDest.setText(folder);
			SetFocus(_txtDest.hWnd());
		}
		return TRUE;
	});

	on_command({RAD_MP3, RAD_FLAC, RAD_WAV}, [this]()->INT_PTR
	{
		_radMp3Cbr.enable(_radMp3.isChecked());
		_cmbCbr.enable(_radMp3.isChecked() && _radMp3Cbr.isChecked());

		_radMp3Vbr.enable(_radMp3.isChecked());
		_cmbVbr.enable(_radMp3.isChecked() && _radMp3Vbr.isChecked());

		EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), _radFlac.isChecked());
		_cmbFlac.enable(_radFlac.isChecked());
		return TRUE;
	});

	on_command({RAD_CBR, RAD_VBR}, [this]()->INT_PTR
	{
		_cmbCbr.enable(_radMp3Cbr.isChecked());
		_cmbVbr.enable(_radMp3Vbr.isChecked());
		return TRUE;
	});

	on_command(BTN_RUN, [this]()->INT_PTR
	{
		vector<wstring> files;
		if (!_destFolderIsOk() || !_filesExist(files)) {
			return TRUE;
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
		DlgRunnin::Target targetType = DlgRunnin::Target::NONE;
	
		if (_radMp3.isChecked())       targetType = DlgRunnin::Target::MP3;
		else if (_radFlac.isChecked()) targetType = DlgRunnin::Target::FLAC;
		else if (_radWav.isChecked())  targetType = DlgRunnin::Target::WAV;

		// Finally invoke dialog.
		DlgRunnin rd(_taskBar, numThreads, targetType,
			files, delSrc, isVbr, quality, _ini,
			_txtDest.getText());
		rd.show(hwnd());
		return TRUE;
	});

	on_notify(LST_FILES, LVN_INSERTITEM, [this](NMHDR& nm)->INT_PTR { return _doUpdateCounter(_lstFiles.items.count()); }); // new item inserted
	on_notify(LST_FILES, LVN_DELETEITEM, [this](NMHDR& nm)->INT_PTR { return _doUpdateCounter(_lstFiles.items.count() - 1); }); // item about to be deleted
	on_notify(LST_FILES, LVN_DELETEALLITEMS, [this](NMHDR& nm)->INT_PTR { return _doUpdateCounter(0); }); // all items about to be deleted

	on_notify(LST_FILES, LVN_KEYDOWN, [this](NMHDR& nm)->INT_PTR
	{
		NMLVKEYDOWN& nkd = reinterpret_cast<NMLVKEYDOWN&>(nm);
		if (nkd.wVKey == VK_DELETE) { // Del key
			SendMessage(hwnd(), WM_COMMAND, MAKEWPARAM(MNU_REMSELECTED, 0), 0);
		}
		return TRUE;
	});
}

bool DlgMain::_destFolderIsOk()
{
	wstring destFolder = _txtDest.getText();
	if (!destFolder.empty()) {
		if (!File::exists(destFolder)) {
			int q = Sys::msgBox(hwnd(), L"Create directory",
				Str::format(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				wstring err;
				if (!File::createDir(destFolder, &err)) {
					Sys::msgBox(hwnd(), L"Fail",
						Str::format(L"The directory failed to be created:\n%s\n%s", destFolder.c_str(), err.c_str()),
						MB_ICONERROR);
					return false; // halt
				}
			} else { // user didn't want to create the new dir
				return false; // halt
			}
		} else if (!File::isDir(destFolder)) {
			Sys::msgBox(hwnd(), L"Fail",
				Str::format(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

bool DlgMain::_filesExist(vector<wstring>& files)
{
	vector<ListView::Item> allItems = _lstFiles.items.getAll();
	files = ListView::getAllText(allItems, 0);

	for (const wstring& f : files) { // each filepath
		if (!File::exists(f)) {
			Sys::msgBox(hwnd(), L"Fail",
				Str::format(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

LRESULT DlgMain::_doUpdateCounter(int newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		Str::format(L"&Run (%d)", newCount) : L"&Run";

	SetWindowText(GetDlgItem(hwnd(), BTN_RUN), caption.c_str());
	EnableWindow(GetDlgItem(hwnd(), BTN_RUN), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void DlgMain::_doFileToList(const wstring& file)
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