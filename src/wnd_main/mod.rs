use std::cell::Cell;
use std::rc::Rc;
use winsafe::{self as w, gui};

mod wnd_main_events;
mod wnd_main_funcs;
mod wnd_main_menu;

#[derive(Clone)]
pub struct WndMain {
	wnd:       gui::WindowMain,
	lst_files: gui::ListView,

	lbl_dest: gui::Label,
	txt_dest: gui::Edit,
	btn_dest: gui::Button,

	fra_conversion:   gui::Label,
	rad_mp3_flac_wav: gui::RadioGroup,
	rad_cbr_vbr:      gui::RadioGroup,
	cmb_cbr:          gui::ComboBox,
	cmb_vbr:          gui::ComboBox,
	lbl_flac_lvl:     gui::Label,
	cmb_flac_lvl:     gui::ComboBox,

	chk_del_orig: gui::CheckBox,
	lbl_threads:  gui::Label,
	cmb_threads:  gui::ComboBox,
	btn_run:      gui::Button,

	resz:   gui::Resizer,
	min_sz: Rc<Cell<w::SIZE>>,
}
