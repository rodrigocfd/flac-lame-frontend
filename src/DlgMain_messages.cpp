
#include "DlgMain.h"
#include <winlamb/sysdlg.h>
#include <winlamb/version.h>
#include "DlgRunnin.h"
#include "../res/resource.h"
using std::vector;
using std::wstring;
using namespace wl;

void DlgMain::messages()
{
	on_message(WM_INITDIALOG, [&](params)
	{
		try {
			validateIni();
		} catch (const std::exception& e) {
			sysdlg::msgbox(this, L"Fail", str::to_wstring(e.what()), MB_ICONERROR);
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // halt program
			return TRUE;
		}

		mTaskbarProg.init(this);
		mTxtDest.assign(this, TXT_DEST);

		// Main listview initialization.
		mLstFiles.assign(this, LST_FILES)
			.set_context_menu(MEN_MAIN)
			.columns.add(L"File", 300)
				.set_width_to_fill(0);
		mLstFiles.imageList16.load_from_shell({L"mp3", L"flac", L"wav"}); // icons of the 3 filetypes we use

		// Initializing comboboxes.
		mCmbCbr.assign(this, CMB_CBR)
			.add(L"32 kbps|40 kbps|48 kbps|56 kbps|64 kbps|80 kbps|96 kbps|"
				L"112 kbps|128 kbps; default|160 kbps|192 kbps|224 kbps|256 kbps|320 kbps")
			.select(8);

		mCmbVbr.assign(this, CMB_VBR)
			.add(L"0 (~245 kbps)|1 (~225 kbps)|2 (~190 kbps)|3 (~175 kbps)|"
				L"4 (~165 kbps); default|5 (~130 kbps)|6 (~115 kbps)|7 (~100 kbps)|"
				L"8 (~85 kbps)|9 (~65 kbps)")
			.select(4);

		mCmbFlac.assign(this, CMB_FLAC)
			.add(L"1|2|3|4|5|6|7|8")
			.select(7);

		mCmbNumThreads.assign(this, CMB_NUMTHREADS)
			.add(L"1|2|4|6|8|12");

		switch (numProcessors()) {
			case 1:  mCmbNumThreads.select(0); break;
			case 2:  mCmbNumThreads.select(1); break;
			case 4:  mCmbNumThreads.select(2); break;
			case 6:  mCmbNumThreads.select(3); break;
			case 8:  mCmbNumThreads.select(4); break;
			case 12: mCmbNumThreads.select(5); break;
			default: mCmbNumThreads.select(0);
		}

		// Initializing radio buttons.
		mRadMFW.assign(this, {RAD_MP3, RAD_FLAC, RAD_WAV});
		mRadMp3Type.assign(this, {RAD_CBR, RAD_VBR});

		mRadMFW.set_checked_by_pos(0);
		mRadMp3Type.set_checked_by_pos(1);

		mChkDelSrc.assign(this, CHK_DELSRC);
		mBtnRun.assign(this, BTN_RUN);

		// Layout control when resizing.
		mResz.add(this, LST_FILES, resizer::go::RESIZE, resizer::go::RESIZE)
			.add(this, TXT_DEST, resizer::go::RESIZE, resizer::go::REPOS)
			.add(this, {LBL_DEST, FRA_CONV, RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR,
				LBL_LEVEL, CMB_CBR, CMB_VBR, CMB_FLAC, CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS},
				resizer::go::NOTHING, resizer::go::REPOS)
			.add(this, {BTN_DEST, BTN_RUN}, resizer::go::REPOS, resizer::go::REPOS);

		return TRUE;
	});

	on_message(WM_SIZE, [&](wm::size p)
	{
		mResz.adjust(p);
		mLstFiles.columns.set_width_to_fill(0);
		return TRUE;
	});

	on_message(WM_DROPFILES, [&](wm::dropfiles p)
	{
		vector<wstring> files = p.files();

		for (const wstring& drop : files) {
			if (file::util::is_dir(drop)) { // if a directory, add all files inside of it
				for (const wstring& f : file::util::list_dir(drop, L"*.mp3")) {
					putFileIntoList(f);
				}
				for (const wstring& f : file::util::list_dir(drop, L"*.flac")) {
					putFileIntoList(f);
				}
				for (const wstring& f : file::util::list_dir(drop, L"*.wav")) {
					putFileIntoList(f);
				}
			} else {
				putFileIntoList(drop); // add single file
			}
		}

		updateCounter(mLstFiles.items.count());
		return TRUE;
	});

	on_message(WM_INITMENUPOPUP, [&](wm::initmenupopup p)
	{
		if (p.first_menu_item_id() == MNU_OPENFILES) {
			menu m = p.hmenu();
			m.enable_item_by_id(MNU_REMSELECTED, mLstFiles.items.count_selected() > 0);
		}
		return TRUE;
	});

	on_command(MNU_ABOUT, [&](params)
	{
		version ver;
		ver.read_current_exe();

		sysdlg::msgbox(this,
			str::format(L"About v%d.%d.%d", ver.num[0], ver.num[1], ver.num[2]),
			L"FLAC/LAME graphical front-end.\n"
			L"Rodrigo C�sar de Freitas Dias.",
			MB_ICONINFORMATION);

		return TRUE;
	});

	on_command(MNU_OPENFILES, [&](params)
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
				putFileIntoList(file);
			}
			updateCounter(mLstFiles.items.count());
		}
		return TRUE;
	});

	on_command(MNU_REMSELECTED, [&](params)
	{
		mLstFiles.items.remove_selected();
		updateCounter(mLstFiles.items.count());
		return TRUE;
	});

	on_command(IDCANCEL, [&](params)
	{
		if (!mLstFiles.items.count() || IsWindowEnabled(mBtnRun.hwnd())) {
			SendMessage(hwnd(), WM_CLOSE, 0, 0); // close on ESC only if not processing
		}
		return TRUE;
	});

	on_command(BTN_DEST, [&](params)
	{
		wstring folder;
		if (sysdlg::choose_folder(this, folder)) {
			mTxtDest.set_text(folder)
				.select_all();
			SetFocus(mTxtDest.hwnd());
		}
		return TRUE;
	});

	on_command({RAD_MP3, RAD_FLAC, RAD_WAV}, [&](params)
	{
		int mfw = mRadMFW.get_checked_id();
		int cv = mRadMp3Type.get_checked_id();

		mRadMp3Type.set_enabled(mfw == RAD_MP3);
		mCmbCbr.set_enabled(mfw == RAD_MP3 && cv == RAD_CBR);
		mCmbVbr.set_enabled(mfw == RAD_MP3 && cv == RAD_VBR);

		EnableWindow(GetDlgItem(hwnd(), LBL_LEVEL), mfw == RAD_FLAC);
		mCmbFlac.set_enabled(mfw == RAD_FLAC);
		return TRUE;
	});

	on_command({RAD_CBR, RAD_VBR}, [&](params)
	{
		int cv = mRadMp3Type.get_checked_id();
		mCmbCbr.set_enabled(cv == RAD_CBR);
		mCmbVbr.set_enabled(cv == RAD_VBR);
		return TRUE;
	});

	on_command(BTN_RUN, [&](params)
	{
		DlgRunnin rd(mTaskbarProg, mIniFile);
		rd.opts.destFolder = mTxtDest.get_text();

		vector<wstring> files;
		try {
			validateDestFolder();
			files = mLstFiles.items.get_texts(mLstFiles.items.get_all(), 0);
			validateFilesExist(files);
		} catch (const std::exception& e) {
			sysdlg::msgbox(this, L"Fail", str::to_wstring(e.what()), MB_ICONERROR);
			return TRUE;
		}
		rd.opts.files = std::move(files);

		// Retrieve settings.
		rd.opts.delSrc = mChkDelSrc.is_checked();
		rd.opts.isVbr = mRadMp3Type.get_checked_id() == RAD_VBR;
		rd.opts.numThreads = std::stoul(mCmbNumThreads.get_selected_text());

		int mfw = mRadMFW.get_checked_id();
		wstring quality;
		if (mfw == RAD_MP3) {
			combobox& cmbQuality = (rd.opts.isVbr ? mCmbVbr : mCmbCbr);
			quality = cmbQuality.get_selected_text();
			quality.resize(quality.find_first_of(L' ')); // first characters of chosen option are the quality setting itself
		} else if (mfw == RAD_FLAC) {
			quality = mCmbFlac.get_selected_text(); // text is quality setting itself
		}
		rd.opts.quality = std::move(quality);

		// Which format are we converting to?
		switch (mfw) {
		case RAD_MP3:  rd.opts.targetType = DlgRunnin::target::MP3; break;
		case RAD_FLAC: rd.opts.targetType = DlgRunnin::target::FLAC; break;
		case RAD_WAV:  rd.opts.targetType = DlgRunnin::target::WAV;
		}

		// Finally invoke dialog.
		rd.show(this);
		return TRUE;
	});

	on_notify(LST_FILES, LVN_INSERTITEM, [&](params)
	{
		return updateCounter(mLstFiles.items.count()); // new item inserted
	});

	on_notify(LST_FILES, LVN_DELETEITEM, [&](params)
	{
		return updateCounter(mLstFiles.items.count() - 1); // item about to be deleted
	});

	on_notify(LST_FILES, LVN_DELETEALLITEMS, [&](params)
	{
		return updateCounter(0); // all items about to be deleted
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