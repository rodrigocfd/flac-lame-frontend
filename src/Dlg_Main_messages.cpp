
#include "Dlg_Main.h"
#include <winlamb/sysdlg.h>
#include <winlamb/thread.h>
#include <winlamb/version.h>
#include "Dlg_Runnin.h"
#include "../res/resource.h"
using namespace wl;

void Dlg_Main::messages()
{
	on_message(WM_INITDIALOG, [&](wm::initdialog)
	{
		try {
			validate_ini();
		} catch (const std::exception& e) {
			sysdlg::msgbox(this, L"Fail", str::to_wstring(e.what()), MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		m_taskbarProg.init(this);
		m_txtDest.assign(this, TXT_DEST);

		// Main listview initialization.
		m_lstFiles.assign(this, LST_FILES)
			.set_context_menu(MEN_MAIN)
			.columns.add(L"File", 300)
			.columns.set_width_to_fill(0)
			.imageList16.load_from_shell({L"mp3", L"flac", L"wav"}); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		m_cmbCbr.assign(this, CMB_CBR)
			.item_add(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
				L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps")
			.item_set_selected(8);

		m_cmbVbr.assign(this, CMB_VBR)
			.item_add(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
				L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)|"
				L"8 (~85 kbps)|9 (~65 kbps)")
			.item_set_selected(4);

		m_cmbFlac.assign(this, CMB_FLAC)
			.item_add(L"1|2|3|4|5|6|7|8")
			.item_set_selected(7);

		m_cmbNumThreads.assign(this, CMB_NUMTHREADS)
			.item_add(L"1|2|4|8");

		switch (thread::num_processors()) {
			case 1:  m_cmbNumThreads.item_set_selected(0); break;
			case 2:  m_cmbNumThreads.item_set_selected(1); break;
			case 4:  m_cmbNumThreads.item_set_selected(2); break;
			case 8:  m_cmbNumThreads.item_set_selected(3); break;
			default: m_cmbNumThreads.item_set_selected(0);
		}

		// Initializing radio buttons.
		m_radMp3   .assign(this, RAD_MP3).set_check_and_trigger(true);
		m_radMp3Cbr.assign(this, RAD_CBR);
		m_radMp3Vbr.assign(this, RAD_VBR).set_check_and_trigger(true);
		m_radFlac  .assign(this, RAD_FLAC);
		m_radWav   .assign(this, RAD_WAV);

		m_chkDelSrc.assign(this, CHK_DELSRC);
		m_btnRun.assign(this, BTN_RUN);

		// Layout control when resizing.
		m_resz.add(this, LST_FILES, resizer::go::RESIZE, resizer::go::RESIZE)
			.add(this, TXT_DEST, resizer::go::RESIZE, resizer::go::REPOS)
			.add(this, {LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR,
				LBL_LEVEL, CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS},
				resizer::go::NOTHING, resizer::go::REPOS)
			.add(this, {BTN_DEST, BTN_RUN}, resizer::go::REPOS, resizer::go::REPOS);

		return TRUE;
	});

	on_message(WM_SIZE, [&](wm::size p)
	{
		m_resz.arrange(p);
		m_lstFiles.columns.set_width_to_fill(0);
		return TRUE;
	});

	on_message(WM_DROPFILES, [&](wm::dropfiles p)
	{
		vector<wstring> files = p.files();

		for (const wstring& drop : files) {
			if (file::util::is_dir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : file::util::list_dir(drop, L"*.mp3")) {
					file_to_list(f);
				}
				for (const wstring& f : file::util::list_dir(drop, L"*.flac")) {
					file_to_list(f);
				}
				for (const wstring& f : file::util::list_dir(drop, L"*.wav")) {
					file_to_list(f);
				}
			} else {
				file_to_list(drop); // add single file
			}
		}

		update_counter(m_lstFiles.items.count());
		return TRUE;
	});

	on_message(WM_INITMENUPOPUP, [&](wm::initmenupopup p)
	{
		if (p.first_menu_item_id() == MNU_OPENFILES) {
			menu m = p.hmenu();
			m.enable_item(MNU_REMSELECTED, m_lstFiles.items.count_selected() > 0);
		}
		return TRUE;
	});

	on_command(MNU_ABOUT, [&](wm::command)
	{
		version ver;
		ver.read_current_exe();

		sysdlg::msgbox(this,
			str::format(L"About v%d.%d.%d", ver.num[0], ver.num[1], ver.num[2]),
			L"FLAC/LAME graphical front-end.\n"
			L"Rodrigo César de Freitas Dias.",
			MB_ICONINFORMATION);

		return TRUE;
	});

	on_command(MNU_OPENFILES, [&](wm::command)
	{
		vector<wstring> files;
		if (sysdlg::open_files(this,
			L"Supported audio files (*.mp3, *.flac, *.wav)|*.mp3;*.flac;*.wav|"
			L"MP3 audio files (*.mp3)|*.mp3|"
			L"FLAC audio files (*.flac)|*.flac|"
			L"WAV audio files (*.wav)|*.wav",
			files))
		{
			for (const wstring& file : files) {
				file_to_list(file);
			}
			update_counter(m_lstFiles.items.count());
		}
		return TRUE;
	});

	on_command(MNU_REMSELECTED, [&](wm::command)
	{
		m_lstFiles.items.remove_selected();
		update_counter(m_lstFiles.items.count());
		return TRUE;
	});

	on_command(IDCANCEL, [&](wm::command)
	{
		if (!m_lstFiles.items.count() || m_btnRun.is_enabled()) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return TRUE;
	});

	on_command(BTN_DEST, [&](wm::command)
	{
		wstring folder;
		if (sysdlg::choose_folder(this, folder)) {
			m_txtDest.set_text(folder)
				.selection_set_all()
				.set_focus();
		}
		return TRUE;
	});

	on_command({RAD_MP3, RAD_FLAC, RAD_WAV}, [&](wm::command)
	{
		m_radMp3Cbr.set_enable(m_radMp3.is_checked());
		m_cmbCbr.set_enable(m_radMp3.is_checked() && m_radMp3Cbr.is_checked());

		m_radMp3Vbr.set_enable(m_radMp3.is_checked());
		m_cmbVbr.set_enable(m_radMp3.is_checked() && m_radMp3Vbr.is_checked());

		EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), m_radFlac.is_checked());
		m_cmbFlac.set_enable(m_radFlac.is_checked());
		return TRUE;
	});

	on_command({RAD_CBR, RAD_VBR}, [&](wm::command)
	{
		m_cmbCbr.set_enable(m_radMp3Cbr.is_checked());
		m_cmbVbr.set_enable(m_radMp3Vbr.is_checked());
		return TRUE;
	});

	on_command(BTN_RUN, [&](wm::command)
	{
		Dlg_Runnin rd(m_taskbarProg, m_iniFile);
		rd.opts.destFolder = m_txtDest.get_text();

		vector<wstring> files;
		try {
			validate_dest_folder();
			files = m_lstFiles.items.get_texts(m_lstFiles.items.get_all(), 0);
			validate_files_exist(files);
		} catch (const std::exception& e) {
			sysdlg::msgbox(this, L"Fail", str::to_wstring(e.what()), MB_ICONERROR);
			return TRUE;
		}
		rd.opts.files = std::move(files);

		// Retrieve settings.
		rd.opts.delSrc = m_chkDelSrc.is_checked();
		rd.opts.isVbr = m_radMp3Vbr.is_checked();
		rd.opts.numThreads = std::stoul(m_cmbNumThreads.item_get_selected_text());

		wstring quality;
		if (m_radMp3.is_checked()) {
			combobox& cmbQuality = (rd.opts.isVbr ? m_cmbVbr : m_cmbCbr);
			quality = cmbQuality.item_get_selected_text();
			quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
		} else if (m_radFlac.is_checked()) {
			quality = m_cmbFlac.item_get_selected_text(); // text is quality setting itself
		}
		rd.opts.quality = std::move(quality);

		// Which format are we converting to?
		if (m_radMp3.is_checked())       rd.opts.targetType = Dlg_Runnin::target::MP3;
		else if (m_radFlac.is_checked()) rd.opts.targetType = Dlg_Runnin::target::FLAC;
		else if (m_radWav.is_checked())  rd.opts.targetType = Dlg_Runnin::target::WAV;

		// Finally invoke dialog.
		rd.show(this);
		return TRUE;
	});

	on_notify(LST_FILES, LVN_INSERTITEM, [&](wmn::lvn::insertitem)
	{
		return update_counter(m_lstFiles.items.count()); // new item inserted
	});

	on_notify(LST_FILES, LVN_DELETEITEM, [&](wmn::lvn::deleteitem)
	{
		return update_counter(m_lstFiles.items.count() - 1); // item about to be deleted
	});

	on_notify(LST_FILES, LVN_DELETEALLITEMS, [&](wmn::lvn::deleteallitems)
	{
		return update_counter(0); // all items about to be deleted
	});

	on_notify(LST_FILES, LVN_KEYDOWN, [&](wmn::lvn::keydown p)
	{
		if (p.nmhdr().wVKey == VK_DELETE) {
			SendMessage(hwnd(), WM_COMMAND, MAKEWPARAM(MNU_REMSELECTED, 0), 0);
			return TRUE;
		}
		return FALSE;
	});
}