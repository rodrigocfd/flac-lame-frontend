
#include "Dlg_Main.h"
#include "Dlg_Runnin.h"
#include "Convert.h"
#include "../winlamb/file.h"
#include "../winlamb/menu.h"
#include "../winlamb/path.h"
#include "../winlamb/str.h"
#include "../winlamb/sys.h"
#include "../winlamb/sysdlg.h"
#include "../res/resource.h"
using namespace wl;
using std::vector;
using std::wstring;

RUN(Dlg_Main);

Dlg_Main::Dlg_Main()
{
	setup.dialogId = DLG_MAIN;
	setup.iconId = ICO_MAIN;
	setup.accelTableId = ACC_MAIN;

	on_message(WM_INITDIALOG, [&](params&)
	{
		if (!preliminar_checks()) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		m_taskbarProg.init(this);
		m_txtDest.be(this, TXT_DEST);

		// Main listview initialization.
		m_lstFiles.be(this, LST_FILES)
			.set_context_menu(MEN_MAIN)
			.column_add(L"File", 300)
			.column_fit(0)
			.icon_push(L"mp3")
			.icon_push(L"flac")
			.icon_push(L"wav"); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		m_cmbCbr.be(this, CMB_CBR)
			.item_add(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
				L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps")
			.item_set_selected(8);

		m_cmbVbr.be(this, CMB_VBR)
			.item_add(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
				L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)|"
				L"8 (~85 kbps)|9 (~65 kbps)")
			.item_set_selected(4);

		m_cmbFlac.be(this, CMB_FLAC)
			.item_add(L"1|2|3|4|5|6|7|8")
			.item_set_selected(7);

		m_cmbNumThreads.be(this, CMB_NUMTHREADS)
			.item_add(L"1|2|4|8");

		switch (num_processors()) {
			case 1:  m_cmbNumThreads.item_set_selected(0); break;
			case 2:  m_cmbNumThreads.item_set_selected(1); break;
			case 4:  m_cmbNumThreads.item_set_selected(2); break;
			case 8:  m_cmbNumThreads.item_set_selected(3); break;
			default: m_cmbNumThreads.item_set_selected(0);
		}

		// Initializing radio buttons.
		m_radMp3   .be(this, RAD_MP3).set_check_and_trigger(true);
		m_radMp3Cbr.be(this, RAD_CBR);
		m_radMp3Vbr.be(this, RAD_VBR).set_check_and_trigger(true);
		m_radFlac  .be(this, RAD_FLAC);
		m_radWav   .be(this, RAD_WAV);

		m_chkDelSrc.be(this, CHK_DELSRC);

		// Layout control when resizing.
		m_resz.add(this, LST_FILES, resizer::go::RESIZE, resizer::go::RESIZE)
			.add(this, TXT_DEST, resizer::go::RESIZE, resizer::go::REPOS)
			.add(this, { LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, LBL_LEVEL,
				CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS },
				resizer::go::NOTHING, resizer::go::REPOS)
			.add(this, { BTN_DEST, BTN_RUN }, resizer::go::REPOS, resizer::go::REPOS);

		return TRUE;
	});

	on_message(WM_SIZE, [&](params& p)
	{
		m_resz.arrange(p);
		m_lstFiles.column_fit(0);
		return TRUE;
	});

	on_message(WM_DROPFILES, [&](wm::dropfiles p)
	{
		vector<wstring> files = p.files();

		for (const wstring& drop : files) {
			if (file::is_dir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.mp3")) {
					file_to_list(f);
				}
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.flac")) {
					file_to_list(f);
				}
				for (const wstring& f : file::list_dir(drop.c_str(), L"*.wav")) {
					file_to_list(f);
				}
			} else {
				file_to_list(drop); // add single file
			}
		}

		update_counter( m_lstFiles.items.count() );
		return TRUE;
	});

	on_message(WM_INITMENUPOPUP, [&](wm::initmenupopup p)
	{
		menu m = p.hmenu();
		if (m.get_command_id(0) == MNU_OPENFILES) {
			m.enable_item(MNU_REMSELECTED, m_lstFiles.items.count_selected() > 0);
			return TRUE;
		}
		return FALSE;
	});

	on_command(MNU_ABOUT, [&](params&)
	{
		sysdlg::msgbox(this, L"About",
			L"FLAC/LAME graphical front-end.", MB_ICONINFORMATION);
		return TRUE;
	});

	on_command(MNU_OPENFILES, [&](params&)
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
				file_to_list(file);
			}
			update_counter( m_lstFiles.items.count() );
		}
		return TRUE;
	});

	on_command(MNU_REMSELECTED, [&](params&)
	{
		m_lstFiles.items.remove_selected();
		update_counter( m_lstFiles.items.count() );
		return TRUE;
	});

	on_command(IDCANCEL, [&](params&)
	{
		if (!m_lstFiles.items.count() || IsWindowEnabled(GetDlgItem(hwnd(), BTN_RUN))) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return TRUE;
	});

	on_command(BTN_DEST, [&](params&)
	{
		wstring folder;
		if (sysdlg::choose_folder(this, folder)) {
			m_txtDest.set_text(folder)
				.selection_set_all()
				.focus();
		}
		return TRUE;
	});

	on_command({RAD_MP3, RAD_FLAC, RAD_WAV}, [&](params&)
	{
		m_radMp3Cbr.enable(m_radMp3.is_checked());
		m_cmbCbr.enable(m_radMp3.is_checked() && m_radMp3Cbr.is_checked());

		m_radMp3Vbr.enable(m_radMp3.is_checked());
		m_cmbVbr.enable(m_radMp3.is_checked() && m_radMp3Vbr.is_checked());

		EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), m_radFlac.is_checked());
		m_cmbFlac.enable(m_radFlac.is_checked());
		return TRUE;
	});

	on_command({RAD_CBR, RAD_VBR}, [&](params&)
	{
		m_cmbCbr.enable(m_radMp3Cbr.is_checked());
		m_cmbVbr.enable(m_radMp3Vbr.is_checked());
		return TRUE;
	});

	on_command(BTN_RUN, [&](params&)
	{
		vector<wstring> files;
		if (!dest_folder_is_ok() || !files_exist(files)) {
			return TRUE;
		}

		// Retrieve settings.
		bool delSrc = m_chkDelSrc.is_checked();
		bool isVbr = m_radMp3Vbr.is_checked();
		int numThreads = std::stoi(m_cmbNumThreads.item_get_selected_text());

		wstring quality;
		if (m_radMp3.is_checked()) {
			combo& cmbQuality = (isVbr ? m_cmbVbr : m_cmbCbr);
			quality = cmbQuality.item_get_selected_text();
			quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
		} else if (m_radFlac.is_checked()) {
			quality = m_cmbFlac.item_get_selected_text(); // text is quality setting itself
		}

		// Which format are we converting to?
		Dlg_Runnin::target targetType = Dlg_Runnin::target::NONE;

		if (m_radMp3.is_checked())       targetType = Dlg_Runnin::target::MP3;
		else if (m_radFlac.is_checked()) targetType = Dlg_Runnin::target::FLAC;
		else if (m_radWav.is_checked())  targetType = Dlg_Runnin::target::WAV;

		// Finally invoke dialog.
		Dlg_Runnin rd(m_taskbarProg, numThreads, targetType,
			files, delSrc, isVbr, quality, m_iniFile,
			m_txtDest.get_text());
		rd.show(this);
		return TRUE;
	});

	on_notify(LST_FILES, LVN_INSERTITEM, [&](params&)
	{
		return update_counter(m_lstFiles.items.count()); // new item inserted
	});

	on_notify(LST_FILES, LVN_DELETEITEM, [&](params&)
	{
		return update_counter(m_lstFiles.items.count() - 1); // item about to be deleted
	});

	on_notify(LST_FILES, LVN_DELETEALLITEMS, [&](params&)
	{
		return update_counter(0); // all items about to be deleted
	});

	on_notify(LST_FILES, LVN_KEYDOWN, [&](listview::notif::keydown p)
	{
		if (p->wVKey == VK_DELETE) {
			SendMessage(hwnd(), WM_COMMAND, MAKEWPARAM(MNU_REMSELECTED, 0), 0);
			return TRUE;
		}
		return FALSE;
	});
}

bool Dlg_Main::preliminar_checks()
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
	if (!m_iniFile.load_from_file(iniPath, &err)) {
		sysdlg::msgbox(this, L"Fail",
			str::format(L"Failed to load:\n%s\n%s", iniPath.c_str(), err.c_str()),
			MB_ICONERROR);
		return false;
	}

	// Validate tools.
	if (!Convert::paths_are_valid(m_iniFile, &err)) {
		sysdlg::msgbox(this, L"Fail", err, MB_ICONERROR);
		return false;
	}

	return true;
}

DWORD Dlg_Main::num_processors() const
{
	SYSTEM_INFO si = { 0 };
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}

bool Dlg_Main::dest_folder_is_ok()
{
	wstring destFolder = m_txtDest.get_text();
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

bool Dlg_Main::files_exist(vector<wstring>& files)
{
	vector<listview::item> allItems = m_lstFiles.items.get_all();
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

INT_PTR Dlg_Main::update_counter(size_t newCount)
{
	// Update counter on Run button.
	wstring caption = newCount ?
		str::format(L"&Run (%d)", newCount) : L"&Run";

	SetWindowText(GetDlgItem(hwnd(), BTN_RUN), caption.c_str());
	EnableWindow(GetDlgItem(hwnd(), BTN_RUN), newCount > 0); // Run button enabled if at least 1 file
	return 0;
};

void Dlg_Main::file_to_list(const wstring& file)
{
	int iType = -1;
	if (path::has_extension(file, L".mp3"))       iType = 0;
	else if (path::has_extension(file, L".flac")) iType = 1;
	else if (path::has_extension(file, L".wav"))  iType = 2; // what type of audio file is this?

	if (iType == -1) {
		return; // bypass file if unaccepted format
	}
	if (!m_lstFiles.items.exists(file)) {
		m_lstFiles.items.add(file, iType); // add only if not present yet
	}
}