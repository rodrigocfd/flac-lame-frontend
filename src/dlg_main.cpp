
#include "dlg_main.h"
#include "dlg_runnin.h"
#include "convert.h"
#include "../winutil/file.h"
#include "../winutil/menu.h"
#include "../winutil/path.h"
#include "../winutil/str.h"
#include "../winutil/sys.h"
#include "../res/resource.h"
using namespace winutil;
using std::vector;
using std::wstring;

RUN(dlg_main);

dlg_main::dlg_main()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;

	on_message(WM_INITDIALOG, [this](params p)->INT_PTR
	{
		// Validate and load INI file.
		wstring iniPath = sys::path_of_exe().append(L"\\FlacLameFE.ini");
		if (!file::exists(iniPath)) {
			sys::msg_box(hwnd(), L"Fail",
				str::format(L"File not found:\n%s", iniPath.c_str()),
				MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		wstring err;
		if (!_ini.load_from_file(iniPath, &err)) {
			sys::msg_box(hwnd(), L"Fail",
				str::format(L"Failed to load:\n%s\n%s", iniPath.c_str(), err.c_str()),
				MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		// Validate tools.
		if (!Convert::pathsAreValid(_ini, &err)) {
			sys::msg_box(hwnd(), L"Fail", err, MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		_taskBar.init(hwnd());
		_txtDest = GetDlgItem(hwnd(), TXT_DEST);

		// Main listview initialization.
		(_lstFiles = GetDlgItem(hwnd(), LST_FILES))
			.set_context_menu(MEN_MAIN)
			.column_add(L"File", 300)
			.column_fit(0)
			.icon_push(L"mp3")
			.icon_push(L"flac")
			.icon_push(L"wav"); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		(_cmbCbr = GetDlgItem(hwnd(), CMB_CBR))
			.item_add(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
				L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps")
			.item_set_selected(8);

		(_cmbVbr = GetDlgItem(hwnd(), CMB_VBR))
			.item_add(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
				L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)|"
				L"8 (~85 kbps)|9 (~65 kbps)")
			.item_set_selected(4);

		(_cmbFlac = GetDlgItem(hwnd(), CMB_FLAC))
			.item_add(L"1|2|3|4|5|6|7|8")
			.item_set_selected(7);

		(_cmbNumThreads = GetDlgItem(hwnd(), CMB_NUMTHREADS))
			.item_add(L"1|2|4|8");

		SYSTEM_INFO si = { 0 };
		GetSystemInfo(&si);
		switch (si.dwNumberOfProcessors) {
			case 1: _cmbNumThreads.item_set_selected(0); break;
			case 2: _cmbNumThreads.item_set_selected(1); break;
			case 4: _cmbNumThreads.item_set_selected(2); break;
			case 8: _cmbNumThreads.item_set_selected(3); break;
			default: _cmbNumThreads.item_set_selected(0);
		}

		// Initializing radio buttons.
		(_radMp3    = GetDlgItem(hwnd(), RAD_MP3)).set_check_and_trigger(true);
		_radMp3Cbr  = GetDlgItem(hwnd(), RAD_CBR);
		(_radMp3Vbr = GetDlgItem(hwnd(), RAD_VBR)).set_check_and_trigger(true);
		_radFlac    = GetDlgItem(hwnd(), RAD_FLAC);
		_radWav     = GetDlgItem(hwnd(), RAD_WAV);

		_chkDelSrc = GetDlgItem(hwnd(), CHK_DELSRC);

		// Layout control when resizing.
		_resizer.add(hwnd(), LST_FILES, resizer::go::RESIZE, resizer::go::RESIZE)
			.add(hwnd(), TXT_DEST, resizer::go::RESIZE, resizer::go::REPOS)
			.add(hwnd(), { LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
				CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
				resizer::go::NOTHING, resizer::go::REPOS)
			.add(hwnd(), { BTN_DEST, BTN_RUN }, resizer::go::REPOS, resizer::go::REPOS);

		sys::set_wheel_hover_behavior(hwnd());
		return TRUE;
	});

	on_message(WM_SIZE, [this](params p)->INT_PTR
	{
		_resizer.arrange(p.wParam, p.lParam);
		_lstFiles.column_fit(0);
		return TRUE;
	});

	on_dropfiles([this](params_dropfiles p)->INT_PTR
	{
		vector<wstring> files = p.get_dropped_files();

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
	});

	on_initmenupopup(MNU_ADDFILES, [this](params_initmenupopup p)->INT_PTR
	{
		menu menu = p.hmenu();
		menu.enable_item(MNU_REMSELECTED, _lstFiles.items.count_selected() > 0);
		return TRUE;
	});

	on_command(MNU_ABOUT, [this](params_command p)->INT_PTR
	{
		sys::msg_box(hwnd(), L"About",
			L"FLAC/LAME graphical front-end.", MB_ICONINFORMATION);
		return TRUE;
	});

	on_command(MNU_ADDFILES, [this](params_command p)->INT_PTR
	{
		vector<wstring> files;
		if (sys::show_open_file(hwnd(),
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
	});

	on_command(MNU_REMSELECTED, [this](params_command p)->INT_PTR
	{
		_lstFiles.items.remove_selected();
		_update_counter( _lstFiles.items.count() );
		return TRUE;
	});

	on_command(IDCANCEL, [this](params_command p)->INT_PTR
	{
		if (!_lstFiles.items.count() || IsWindowEnabled(GetDlgItem(hwnd(), BTN_RUN))) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return TRUE;
	});

	on_command(BTN_DEST, [this](params_command p)->INT_PTR
	{
		wstring folder;
		if (sys::show_choose_folder(hwnd(), folder)) {
			_txtDest.set_text(folder);
			SetFocus(_txtDest.hwnd());
		}
		return TRUE;
	});

	on_command({RAD_MP3, RAD_FLAC, RAD_WAV}, [this](params_command p)->INT_PTR
	{
		_radMp3Cbr.enable(_radMp3.is_checked());
		_cmbCbr.enable(_radMp3.is_checked() && _radMp3Cbr.is_checked());

		_radMp3Vbr.enable(_radMp3.is_checked());
		_cmbVbr.enable(_radMp3.is_checked() && _radMp3Vbr.is_checked());

		EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), _radFlac.is_checked());
		_cmbFlac.enable(_radFlac.is_checked());
		return TRUE;
	});

	on_command({RAD_CBR, RAD_VBR}, [this](params_command p)->INT_PTR
	{
		_cmbCbr.enable(_radMp3Cbr.is_checked());
		_cmbVbr.enable(_radMp3Vbr.is_checked());
		return TRUE;
	});

	on_command(BTN_RUN, [this](params_command p)->INT_PTR
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
			combobox& cmbQuality = (isVbr ? _cmbVbr : _cmbCbr);
			quality = cmbQuality.item_get_selected_text();
			quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
		} else if (_radFlac.is_checked()) {
			quality = _cmbFlac.item_get_selected_text(); // text is quality setting itself
		}

		// Which format are we converting to?
		dlg_runnin::target targetType = dlg_runnin::target::NONE;
	
		if (_radMp3.is_checked())       targetType = dlg_runnin::target::MP3;
		else if (_radFlac.is_checked()) targetType = dlg_runnin::target::FLAC;
		else if (_radWav.is_checked())  targetType = dlg_runnin::target::WAV;

		// Finally invoke dialog.
		dlg_runnin rd(_taskBar, numThreads, targetType,
			files, delSrc, isVbr, quality, _ini,
			_txtDest.get_text());
		rd.show(hwnd());
		return TRUE;
	});

	on_notify(LST_FILES, LVN_INSERTITEM, [this](params_notify p)->INT_PTR
	{
		return _update_counter(_lstFiles.items.count()); // new item inserted
	});

	on_notify(LST_FILES, LVN_DELETEITEM, [this](params_notify p)->INT_PTR
	{
		return _update_counter(_lstFiles.items.count() - 1); // item about to be deleted
	});

	on_notify(LST_FILES, LVN_DELETEALLITEMS, [this](params_notify p)->INT_PTR
	{
		return _update_counter(0); // all items about to be deleted
	});

	on_notify(LST_FILES, LVN_KEYDOWN, [this](params_notify p)->INT_PTR
	{
		NMLVKEYDOWN& nkd = reinterpret_cast<NMLVKEYDOWN&>(p.nmhdr());
		if (nkd.wVKey == VK_DELETE) { // Del key
			SendMessage(hwnd(), WM_COMMAND, MAKEWPARAM(MNU_REMSELECTED, 0), 0);
		}
		return TRUE;
	});
}

bool dlg_main::_dest_folder_is_ok()
{
	wstring destFolder = _txtDest.get_text();
	if (!destFolder.empty()) {
		if (!file::exists(destFolder)) {
			int q = sys::msg_box(hwnd(), L"Create directory",
				str::format(L"The following directory:\n%s\ndoes not exist. Create it?", destFolder.c_str()),
				MB_ICONQUESTION | MB_YESNO);
			if (q == IDYES) {
				wstring err;
				if (!file::create_dir(destFolder, &err)) {
					sys::msg_box(hwnd(), L"Fail",
						str::format(L"The directory failed to be created:\n%s\n%s", destFolder.c_str(), err.c_str()),
						MB_ICONERROR);
					return false; // halt
				}
			} else { // user didn't want to create the new dir
				return false; // halt
			}
		} else if (!file::is_dir(destFolder)) {
			sys::msg_box(hwnd(), L"Fail",
				str::format(L"The following path is not a directory:\n%s", destFolder.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

bool dlg_main::_files_exist(vector<wstring>& files)
{
	vector<listview::item> allItems = _lstFiles.items.get_all();
	files = listview::get_all_text(allItems, 0);

	for (const wstring& f : files) { // each filepath
		if (!file::exists(f)) {
			sys::msg_box(hwnd(), L"Fail",
				str::format(L"Process aborted, file does not exist:\n%s", f.c_str()),
				MB_ICONERROR);
			return false; // halt
		}
	}
	return true;
}

LRESULT dlg_main::_update_counter(size_t newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		str::format(L"&Run (%d)", newCount) : L"&Run";

	SetWindowText(GetDlgItem(hwnd(), BTN_RUN), caption.c_str());
	EnableWindow(GetDlgItem(hwnd(), BTN_RUN), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void dlg_main::_file_to_list(const wstring& file)
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