#include <crtdbg.h>
#include "dlg_main.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int cmdShow) {
	int ret = 0;
	{
		auto ole = wd::OleInit{};
		DlgMain dlg{};
		ret = wd::run_main_dialog(hInst, cmdShow, dlg, {
			{wd::Acc::Key::ctrl, 'O', MNU_OPENFILES},
			{wd::Acc::Key::none, VK_F1, MNU_ABOUT},
		});
	}
	if (_CrtDumpMemoryLeaks())
		MessageBoxW(nullptr, L"A memory leak was found.", L"Memory leak", MB_ICONERROR);
	return ret;
}

bool DlgMain::on_init_dialog() {
	RECT rc{};
	GetWindowRect(hwnd(), &rc); // in screen coords
	_szWndOrig.x = rc.right - rc.left; // for MINMAXINFO
	_szWndOrig.y = rc.bottom - rc.top;

	mnuFiles.create_popup()
		.add(L"&Open files...\tCtrl+O", MNU_OPENFILES)
		.add(L"&Remove selected\tDel", MNU_REMSELECTED)
		.add_sep()
		.add(L"&About...\tF1", MNU_ABOUT);
	
	_imgList.create16()
		.add_file_ext({L"mp3", L"flac", L"wav"});

	lstFiles.activate_mods(mnuFiles)
		.set_full_row_sel()
		.set_image_list16(_imgList)
		.col_add(L"File", 1)
		.col_add(L"Size", wd::dpi::x(70))
		.sort({_lstFilesSort.col, _lstFilesSort.asc})
		.col(0).set_width_to_fill();
	lstFiles.col(1).set_align(wd::ListView::Align::center);

	cmbCbr.item_add({
		L"32 kbps", L"40 kbps", L"48 kbps", L"56 kbps",
		L"64 kbps", L"80 kbps", L"96 kbps", L"112 kbps",
		L"128 kbps; default",
		L"160 kbps", L"192 kbps", L"224 kbps", L"256 kbps", L"320 kbps",
	});
	cmbVbr.item_add({
		L"0 (~245 kbps)", L"1 (~225 kbps)", L"2 (~190 kbps)", L"3 (~175 kbps)",
		L"4 (~165 kbps); default",
		L"5 (~130 kbps)", L"6 (~115 kbps)", L"7 (~100 kbps)", L"8 (~85 kbps)", L"9 (~65 kbps)",
	});
	cmbFlac.item_add({L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8"});
	set_initial_number_of_threads();

	layout.add(LST_FILES, wd::Lay::resz_resz)
		.add(LBL_DEST, wd::Lay::hold_move)
		.add(TXT_DEST, wd::Lay::resz_move)
		.add(BTN_DEST, wd::Lay::move_move)
		.add({FRA_CONV,
			RAD_MP3, RAD_FLAC, RAD_WAV, RAD_CBR, RAD_VBR, CMB_CBR, CMB_VBR, LBL_LEVEL, CMB_FLAC,
			CHK_DELSRC, LBL_NUMTHREADS, CMB_NUMTHREADS,
		}, wd::Lay::hold_move)
		.add(BTN_RUN, wd::Lay::move_move);

	lstFilesDropTarget.register_drag_drop(lstFiles)
		.on_drag_enter(std::bind(&DlgMain::on_drag_files, this, std::placeholders::_1))
		.on_drop(std::bind(&DlgMain::on_drop_files, this, std::placeholders::_1));

	if (!load_ini_settings()) [[unlikely]] {
		PostMessageW(hwnd(), WM_CLOSE, 0, 0);
	}
	return true;
}

bool DlgMain::on_get_min_max_info(MINMAXINFO &mmi) {
	mmi.ptMinTrackSize = _szWndOrig;
	return true;
}

void DlgMain::on_size(WORD req, SIZE sz) {
	if (req != SIZE_MINIMIZED) [[likely]] {
		layout.rearrange(req, sz);
		lstFiles.col(0).set_width_to_fill();
	}
}

void DlgMain::on_init_menu_popup(HMENU hMenu) {
	if (mnuFiles.hmenu() == hMenu) [[likely]] {
		mnuFiles.enable_cmd(lstFiles.item_selected_count() > 0, MNU_REMSELECTED);
	}
}

bool DlgMain::on_command(WORD id, WORD) {
	switch (id) {
		case MNU_OPENFILES:   menu_open_files(); return true;
		case MNU_REMSELECTED: menu_rem_selected(); return true;
		case MNU_ABOUT:       wd::sys_dlg::msg_about(this, ICO_RONBURGUNDY); return true;
		case BTN_DEST:        btn_dest(); return true;
		case RAD_MP3:
		case RAD_FLAC:
		case RAD_WAV:
		case RAD_CBR:
		case RAD_VBR:         rad_click(); return true;
		case BTN_RUN:         btn_run(); return true;
	}
	return false;
}

bool DlgMain::on_notify(NMHDR &nm) {
	switch (nm.idFrom) {
		case LST_FILES:
			switch (nm.code) {
				case LVN_KEYDOWN:
					switch (reinterpret_cast<const NMLVKEYDOWN&>(nm).wVKey) {
						case VK_DELETE: menu_rem_selected(); return true;
					}
					break;
				case LVN_DELETEITEM: lst_delete_item(reinterpret_cast<const NMLISTVIEW&>(nm)); return true;
				case HDN_ITEMCLICK:  lst_header_click(reinterpret_cast<const NMHEADERW&>(nm)); return true;
			}
			break;
	}
	return false;
}

void DlgMain::on_destroy() {
	save_ini_settings();
	PostQuitMessage(0);
}
