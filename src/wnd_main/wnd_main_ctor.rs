use std::cell::Cell;
use std::rc::Rc;
use winsafe::{self as w, prelude::*, co, gui};

use crate::ids;
use super::WndMain;

impl WndMain {
	pub fn new() -> w::AnyResult<Self> {
		use gui::{Horz as H, Vert as V};

		let wnd = gui::WindowMain::new_dlg(ids::DLG_MAIN, Some(ids::ICO_MAIN), Some(ids::ACC_MAIN));
		let lst_files = gui::ListView::new_dlg(&wnd, ids::LST_FILES, (H::Resize, V::Resize), Some(ids::MEN_MAIN));

		let lbl_dest = gui::Label::new_dlg(&wnd, ids::LBL_DEST, (H::None, V::Repos));
		let txt_dest = gui::Edit::new_dlg(&wnd, ids::TXT_DEST, (H::Resize, V::Repos));
		let btn_dest = gui::Button::new_dlg(&wnd, ids::BTN_DEST, (H::Repos, V::Repos));

		let fra_conversion   = gui::Label::new_dlg(&wnd, ids::FRA_CONVERSION, (H::None, V::Repos));
		let rad_mp3_flac_wav = gui::RadioGroup::new_dlg(&wnd, &[
			(ids::RAD_MP3, H::None, V::Repos),
			(ids::RAD_FLAC, H::None, V::Repos),
			(ids::RAD_WAV, H::None, V::Repos),
		]);
		let rad_cbr_vbr  = gui::RadioGroup::new_dlg(&wnd, &[
			(ids::RAD_CBR, H::None, V::Repos),
			(ids::RAD_VBR, H::None, V::Repos),
		]);
		let cmb_cbr      = gui::ComboBox::new_dlg(&wnd, ids::CMB_CBR, (H::None, V::Repos));
		let cmb_vbr      = gui::ComboBox::new_dlg(&wnd, ids::CMB_VBR, (H::None, V::Repos));
		let lbl_flac_lvl = gui::Label::new_dlg(&wnd, ids::LBL_FLAC_LVL, (H::None, V::Repos));
		let cmb_flac_lvl = gui::ComboBox::new_dlg(&wnd, ids::CMB_FLAC_LVL, (H::None, V::Repos));

		let chk_del_orig = gui::CheckBox::new_dlg(&wnd, ids::CHK_DEL_ORIG, (H::None, V::Repos));
		let lbl_threads  = gui::Label::new_dlg(&wnd, ids::LBL_THREADS, (H::None, V::Repos));
		let cmb_threads  = gui::ComboBox::new_dlg(&wnd, ids::CMB_THREADS, (H::None, V::Repos));
		let btn_run      = gui::Button::new_dlg(&wnd, ids::BTN_RUN, (H::Repos, V::Repos));

		let new_self = Self {
			wnd, lst_files,
			lbl_dest, txt_dest, btn_dest,
			fra_conversion, rad_mp3_flac_wav, rad_cbr_vbr,
			cmb_cbr, cmb_vbr, lbl_flac_lvl, cmb_flac_lvl,
			chk_del_orig, lbl_threads, cmb_threads, btn_run,
			min_sz: Rc::new(Cell::new(w::SIZE::default())),
		};
		new_self.events_wm();
		new_self.events_menu();
		Ok(new_self)
	}

	pub fn run(&self) -> w::AnyResult<i32> {
		self.wnd.run_main(None)
	}

	pub(super) fn init_dialog(&self) -> w::AnyResult<bool> {
		// Since the list view doesn't have LVS_SHAREIMAGELISTS style (not
		// set in the resource editor), the image list will be automatically
		// deleted by the list view.
		let himg = w::HIMAGELIST::Create(w::SIZE::new(16, 16), co::ILC::COLOR32, 3, 1)?.leak();
		himg.add_icon_from_shell(&["mp3", "flac", "wav"])?;
		self.lst_files.set_image_list(co::LVSIL::SMALL, &himg);

		self.lst_files.columns().add(&[("File", 100), ("Size", 70)]);
		self.lst_files.columns().get(0).set_width_to_fill();

		self.cmb_cbr.items().add(&[
			"32 kbps", "40 kbps", "48 kbps", "56 kbps",
			"64 kbps", "80 kbps", "96 kbps", "112 kbps",
			"128 kbps; default",
			"160 kbps", "192 kbps", "224 kbps", "256 kbps", "320 kbps"]);
		self.cmb_cbr.items().select(Some(8));

		self.cmb_vbr.items().add(&[
			"0 (~245 kbps)", "1 (~225 kbps)", "2 (~190 kbps)", "3 (~175 kbps)",
			"4 (~165 kbps); default",
			"5 (~130 kbps)", "6 (~115 kbps)", "7 (~100 kbps)", "8 (~85 kbps)", "9 (~65 kbps)"]);
		self.cmb_vbr.items().select(Some(4));

		self.cmb_flac_lvl.items().add(&["1", "2", "3", "4", "5", "6", "7", "8"]);
		self.cmb_flac_lvl.items().select(Some(7));

		self.rad_mp3_flac_wav[0].select_and_trigger(true)?;
		self.rad_cbr_vbr[1].select_and_trigger(true)?;

		let si = w::GetSystemInfo();

		self.cmb_threads.items().add(&["1", "2", "4", "6", "8", "12"]);
		self.cmb_threads.items().select(
			Some(match si.dwNumberOfProcessors {
				2  => 1,
				4  => 2,
				6  => 3,
				8  => 4,
				12 => 5,
				_ => 0,
			}),
		);

		let rc = self.wnd.hwnd().GetWindowRect()?;
		self.min_sz.replace(w::SIZE::new(rc.right - rc.left, rc.bottom - rc.top - 200));

		Ok(true)
	}
}
