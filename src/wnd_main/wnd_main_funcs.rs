use std::cell::Cell;
use std::error::Error;
use std::rc::Rc;
use winsafe::{self as w, co, gui};

use crate::{ids, util};
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

		let fra_conversion   = gui::Label::new_dlg(&wnd, ids::FRA_CONVERSION);
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

		let resz = gui::Resizer::new(&wnd, &[
			(gui::Resz::Resize, gui::Resz::Resize, &[&lst_files]),
			(gui::Resz::Nothing, gui::Resz::Repos, &[&lbl_dest]),
			(gui::Resz::Resize, gui::Resz::Repos, &[&txt_dest]),
			(gui::Resz::Repos, gui::Resz::Repos, &[&btn_dest]),
			(gui::Resz::Nothing, gui::Resz::Repos, &[
				&fra_conversion,
				&cmb_cbr, &cmb_vbr,
				&lbl_flac_lvl, &cmb_flac_lvl,
				&chk_del_orig, &lbl_threads, &cmb_threads,
			]),
			(gui::Resz::Nothing, gui::Resz::Repos, &rad_mp3_flac_wav.as_child_vec()),
			(gui::Resz::Nothing, gui::Resz::Repos, &rad_cbr_vbr.as_child_vec()),
			(gui::Resz::Repos, gui::Resz::Repos, &[&btn_run]),
		]);

		let new_self = Self {
			wnd, lst_files,
			lbl_dest, txt_dest, btn_dest,
			fra_conversion, rad_mp3_flac_wav, rad_cbr_vbr,
			cmb_cbr, cmb_vbr, lbl_flac_lvl, cmb_flac_lvl,
			chk_del_orig, lbl_threads, cmb_threads, btn_run, resz,
			min_sz: Rc::new(Cell::new(w::SIZE::default())),
		};
		new_self.events();
		new_self.menu_events();
		new_self
	}

	pub fn run(&self) -> w::WinResult<()> {
		self.wnd.run_main(None)
	}

	pub(super) fn update_run_count(&self) -> Result<(), Box<dyn Error>> {
		let num_files = self.lst_files.items().count();
		if num_files == 0 {
			self.btn_run.hwnd().SetWindowText("&Run")?;
		} else {
			self.btn_run.hwnd().SetWindowText(&format!("Run ({})", num_files))?;
		}
		self.btn_run.hwnd().EnableWindow(num_files > 0);
		Ok(())
	}

	pub(super) fn add_files<S: AsRef<str>>(&self, files: &[S]) -> Result<(), Box<dyn Error>> {
		for file in files.iter() {
			let file = file.as_ref();
			let (hfile, _) = w::HFILE::CreateFile(file, co::GENERIC::READ,
				co::FILE_SHARE::READ, None, co::DISPOSITION::OPEN_EXISTING,
				co::FILE_ATTRIBUTE::NORMAL, None)?;
			let sz = hfile.GetFileSizeEx()?;
			hfile.CloseHandle()?;

			self.lst_files.items().add(&[file, &util::format_bytes(sz)],
				Some(if file.starts_with("mp3") {
					0
				} else if file.starts_with("flac") {
					1
				} else { // wav
					2
				}),
			)?;
		}
		self.lst_files.columns().set_width_to_fill(0)?;
		self.update_run_count()?;
		Ok(())
	}
}
