
#include "Dlg_Main.h"
#include "Dlg_Runnin.h"
#include "Convert.h"
#include "../wet/drop_files.h"
#include "../wet/file.h"
#include "../wet/menu.h"
#include "../wet/path.h"
#include "../wet/str.h"
#include "../wet/sys.h"
#include "../wet/sysdlg.h"
#include "../res/resource.h"
using namespace wet;
using std::vector;
using std::wstring;

RUN(Dlg_Main);

Dlg_Main::Dlg_Main()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;
}

INT_PTR Dlg_Main::proc(params p)
{
	switch (p.message) {
	case WM_INITDIALOG: return [&](params p)
	{
		if (!_preliminar_checks()) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		_taskBar.init(this);
		_txtDest.be(this, TXT_DEST);

		// Main listview initialization.
		_lstFiles.be(this, LST_FILES)
			.set_context_menu(MEN_MAIN)
			.column_add(L"File", 300)
			.column_fit(0)
			.icon_push(L"mp3")
			.icon_push(L"flac")
			.icon_push(L"wav"); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		_cmbCbr.be(this, CMB_CBR)
			.item_add(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
				L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps")
			.item_set_selected(8);

		_cmbVbr.be(this, CMB_VBR)
			.item_add(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
				L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)|"
				L"8 (~85 kbps)|9 (~65 kbps)")
			.item_set_selected(4);

		_cmbFlac.be(this, CMB_FLAC)
			.item_add(L"1|2|3|4|5|6|7|8")
			.item_set_selected(7);

		_cmbNumThreads.be(this, CMB_NUMTHREADS)
			.item_add(L"1|2|4|8");

		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		switch (si.dwNumberOfProcessors) {
			case 1:  _cmbNumThreads.item_set_selected(0); break;
			case 2:  _cmbNumThreads.item_set_selected(1); break;
			case 4:  _cmbNumThreads.item_set_selected(2); break;
			case 8:  _cmbNumThreads.item_set_selected(3); break;
			default: _cmbNumThreads.item_set_selected(0);
		}

		// Initializing radio buttons.
		_radMp3   .be(this, RAD_MP3).set_check_and_trigger(true);
		_radMp3Cbr.be(this, RAD_CBR);
		_radMp3Vbr.be(this, RAD_VBR).set_check_and_trigger(true);
		_radFlac  .be(this, RAD_FLAC);
		_radWav   .be(this, RAD_WAV);

		_chkDelSrc.be(this, CHK_DELSRC);

		// Layout control when resizing.
		_resizer.add(this, LST_FILES, resizer::go::RESIZE, resizer::go::RESIZE)
			.add(this, TXT_DEST, resizer::go::RESIZE, resizer::go::REPOS)
			.add(this, { LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
				CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
				resizer::go::NOTHING, resizer::go::REPOS)
			.add(this, { BTN_DEST, BTN_RUN }, resizer::go::REPOS, resizer::go::REPOS);

		return TRUE;
	}(p);

	case WM_SIZE: return [&](params::size p)
	{
		_resizer.arrange(p);
		_lstFiles.column_fit(0);
		return TRUE;
	}(p);

	case WM_DROPFILES: return [&](params::dropfiles p)
	{
		vector<wstring> files = p.drop().files();
		for (const wstring& drop : files) {
			if (file::is_dir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.mp3")) {
					_file_to_list(f);
				}
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.flac")) {
					_file_to_list(f);
				}
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.wav")) {
					_file_to_list(f);
				}
			} else {
				_file_to_list(drop); // add single file
			}
		}
		_update_counter( _lstFiles.items.count() );
		return TRUE;
	}(p);

	case WM_INITMENUPOPUP: return [&](params::initmenupopup p)->INT_PTR
	{
		if (p.menup().get_command_id(0) == MNU_OPENFILES) {
			p.menup().enable_item(MNU_REMSELECTED, _lstFiles.items.count_selected() > 0);
			return TRUE;
		}
		return def_proc(p);
	}(p);

	case WM_COMMAND: return [&](params::command p)->INT_PTR
	{
		switch (p.control_id()) {
		case MNU_ABOUT: return [&](params::command p)
		{
			sysdlg::msgbox(this, L"About",
				L"FLAC/LAME graphical front-end.", MB_ICONINFORMATION);
			return TRUE;
		}(p);

		case MNU_OPENFILES: return [&](params::command p)
		{
			vector<wstring> files;
			if (sysdlg::open_file(this,
				L"Supported audio files (*.mp3, *.flac, *.wav)|*.mp3;*.flac;*.wav|"
				L"MP3 audio files (*.mp3)|*.mp3|"
				L"FLAC audio files (*.flac)|*.flac|"
				L"WAV audio files (*.wav)|*.wav",
				files))
			{
				for (const wstring& file : files) {
					_file_to_list(file);
				}
				_update_counter( _lstFiles.items.count() );
			}
			return TRUE;
		}(p);

		case MNU_REMSELECTED: return [&](params::command p)
		{
			_lstFiles.items.remove_selected();
			_update_counter( _lstFiles.items.count() );
			return TRUE;
		}(p);

		case IDCANCEL: return [&](params::command p)
		{
			if (!_lstFiles.items.count() || IsWindowEnabled(GetDlgItem(hwnd(), BTN_RUN))) {
				SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
			}
			return TRUE;
		}(p);

		case BTN_DEST: return [&](params::command p)
		{
			wstring folder;
			if (sysdlg::choose_folder(this, folder)) {
				_txtDest.set_text(folder)
					.selection_set_all()
					.focus();
			}
			return TRUE;
		}(p);

		case RAD_MP3:
		case RAD_FLAC:
		case RAD_WAV: return [&](params::command p)
		{
			_radMp3Cbr.enable(_radMp3.is_checked());
			_cmbCbr.enable(_radMp3.is_checked() && _radMp3Cbr.is_checked());

			_radMp3Vbr.enable(_radMp3.is_checked());
			_cmbVbr.enable(_radMp3.is_checked() && _radMp3Vbr.is_checked());

			EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), _radFlac.is_checked());
			_cmbFlac.enable(_radFlac.is_checked());
			return TRUE;
		}(p);

		case RAD_CBR:
		case RAD_VBR: return [&](params::command p)
		{
			_cmbCbr.enable(_radMp3Cbr.is_checked());
			_cmbVbr.enable(_radMp3Vbr.is_checked());
			return TRUE;
		}(p);

		case BTN_RUN: return [&](params::command p)
		{
			vector<wstring> files;
			if (!_dest_folder_is_ok() || !_files_exist(files)) {
				return TRUE;
			}

			// Retrieve settings.
			bool delSrc = _chkDelSrc.is_checked();
			bool isVbr = _radMp3Vbr.is_checked();
			int numThreads = std::stoi(_cmbNumThreads.item_get_selected_text());
		
			wstring quality;
			if (_radMp3.is_checked()) {
				combo& cmbQuality = (isVbr ? _cmbVbr : _cmbCbr);
				quality = cmbQuality.item_get_selected_text();
				quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
			} else if (_radFlac.is_checked()) {
				quality = _cmbFlac.item_get_selected_text(); // text is quality setting itself
			}

			// Which format are we converting to?
			Dlg_Runnin::target targetType = Dlg_Runnin::target::NONE;
		
			if (_radMp3.is_checked())       targetType = Dlg_Runnin::target::MP3;
			else if (_radFlac.is_checked()) targetType = Dlg_Runnin::target::FLAC;
			else if (_radWav.is_checked())  targetType = Dlg_Runnin::target::WAV;

			// Finally invoke dialog.
			Dlg_Runnin rd(_taskBar, numThreads, targetType,
				files, delSrc, isVbr, quality, _ini,
				_txtDest.get_text());
			rd.show(this);
			return TRUE;
		}(p);
		}
		return def_proc(p);
	}(p);

	case WM_NOTIFY: return [&](params::notify p)->INT_PTR
	{
		switch (p.nmhdr().idFrom) {
		case LST_FILES:
			switch (p.nmhdr().code) {
			case LVN_INSERTITEM: return [&](params::notify p)
			{
				return _update_counter(_lstFiles.items.count()); // new item inserted
			}(p);

			case LVN_DELETEITEM: return [&](params::notify p)
			{
				return _update_counter(_lstFiles.items.count() - 1); // item about to be deleted
			}(p);

			case LVN_DELETEALLITEMS: return [&](params::notify p)
			{
				return _update_counter(0); // all items about to be deleted
			}(p);

			case LVN_KEYDOWN:
				switch (p.nmlvkeydown().wVKey) {
				case VK_DELETE: return [&](params::notify p)
				{
					SendMessage(hwnd(), WM_COMMAND, MAKEWPARAM(MNU_REMSELECTED, 0), 0);
					return TRUE;
				}(p);
				}
			}
		}
		return def_proc(p);
	}(p);
	}
	return def_proc(p);
}

bool Dlg_Main::_preliminar_checks()
{
	// Validate and load INI file.
	wstring iniPath = path::exe_path().append(L"\\FlacLameFE.ini");
	if (!file::exists(iniPath)) {
		sysdlg::msgbox(this, L"Fail",
			str::format(L"File not found:\n%s", iniPath.c_str()),
			MB_ICONERROR);
		return false;
	}

	wstring err;
	if (!_ini.load_from_file(iniPath, &err)) {
		sysdlg::msgbox(this, L"Fail",
			str::format(L"Failed to load:\n%s\n%s", iniPath.c_str(), err.c_str()),
			MB_ICONERROR);
		return false;
	}

	// Validate tools.
	if (!Convert::paths_are_valid(_ini, &err)) {
		sysdlg::msgbox(this, L"Fail", err, MB_ICONERROR);
		return false;
	}

	return true;
}

bool Dlg_Main::_dest_folder_is_ok()
{
	wstring destFolder = _txtDest.get_text();
	if (!destFolder.empty()) {
		if (!file::exists(destFolder)) {
			int q = sysdlg::msgbox(this, L"Create directory",
				str::format(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				wstring err;
				if (!file::create_dir(destFolder, &err)) {
					sysdlg::msgbox(this, L"Fail",
						str::format(L"The directory failed to be created:\n%s\n%s", destFolder.c_str(), err.c_str()),
						MB_ICONERROR);
					return false; // halt
				}
			} else { // user didn't want to create the new dir
				return false; // halt
			}
		} else if (!file::is_dir(destFolder)) {
			sysdlg::msgbox(this, L"Fail",
				str::format(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

bool Dlg_Main::_files_exist(vector<wstring>& files)
{
	vector<listview::item> allItems = _lstFiles.items.get_all();
	files = listview::get_all_text(allItems, 0);

	for (const wstring& f : files) { // each filepath
		if (!file::exists(f)) {
			sysdlg::msgbox(this, L"Fail",
				str::format(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

LRESULT Dlg_Main::_update_counter(size_t newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		str::format(L"&Run (%d)", newCount) : L"&Run";

	SetWindowText(GetDlgItem(hwnd(), BTN_RUN), caption.c_str());
	EnableWindow(GetDlgItem(hwnd(), BTN_RUN), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void Dlg_Main::_file_to_list(const wstring& file)
{
	int iType = -1;
	if (path::has_extension(file, L".mp3"))       iType = 0;
	else if (path::has_extension(file, L".flac")) iType = 1;
	else if (path::has_extension(file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!_lstFiles.items.exists(file)) {
		_lstFiles.items.add(file, iType); // add only if not present yet
	}
}