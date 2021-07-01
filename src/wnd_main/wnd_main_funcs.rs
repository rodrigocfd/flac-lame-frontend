use winsafe::{self as w, gui};

use crate::ids;
use super::WndMain;

impl WndMain {
	pub fn new() -> Self {
		let context_menu = w::HINSTANCE::NULL
			.LoadMenu(ids::MEN_MAIN).unwrap()
			.GetSubMenu(0).unwrap();

		let wnd = gui::WindowMain::new_dlg(ids::DLG_MAIN, Some(ids::ICO_MAIN), Some(ids::ACC_MAIN));
		let lst_files = gui::ListView::new_dlg(&wnd, ids::LST_FILES, Some(context_menu));

		let lbl_dest = gui::Label::new_dlg(&wnd, ids::LBL_DEST);
		let txt_dest = gui::Edit::new_dlg(&wnd, ids::TXT_DEST);
		let btn_dest = gui::Button::new_dlg(&wnd, ids::BTN_DEST);

		let fra_bitrate      = gui::Label::new_dlg(&wnd, ids::FRA_BITRATE);
		let rad_mp3_flac_wav = gui::RadioGroup::new_dlg(&wnd, &[ids::RAD_MP3, ids::RAD_FLAC, ids::RAD_WAV]);
		let rad_cbr_vbr      = gui::RadioGroup::new_dlg(&wnd, &[ids::RAD_CBR, ids::RAD_VBR]);
		let cmb_cbr          = gui::ComboBox::new_dlg(&wnd, ids::CMB_CBR);
		let cmb_vbr          = gui::ComboBox::new_dlg(&wnd, ids::CMB_VBR);
		let lbl_flac_lvl     = gui::Label::new_dlg(&wnd, ids::LBL_FLAC_LVL);
		let cmb_flac_lvl     = gui::ComboBox::new_dlg(&wnd, ids::CMB_FLAC_LVL);

		let chk_del_orig = gui::CheckBox::new_dlg(&wnd, ids::CHK_DEL_ORIG);
		let lbl_threads  = gui::Label::new_dlg(&wnd, ids::LBL_THREADS);
		let cmb_threads  = gui::ComboBox::new_dlg(&wnd, ids::CMB_THREADS);
		let btn_run      = gui::Button::new_dlg(&wnd, ids::BTN_RUN);

		let new_self = Self {
			wnd, lst_files,
			lbl_dest, txt_dest, btn_dest,
			fra_bitrate, rad_mp3_flac_wav, rad_cbr_vbr,
			cmb_cbr, cmb_vbr, lbl_flac_lvl, cmb_flac_lvl,
			chk_del_orig, lbl_threads, cmb_threads, btn_run,
		};
		new_self.events();
		new_self
	}

	pub fn run(&self) -> w::WinResult<()> {
		self.wnd.run_main(None)
	}

	pub(super) fn load_ini(&self) {

	}
}
