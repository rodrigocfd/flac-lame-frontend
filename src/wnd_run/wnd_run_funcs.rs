use winsafe::gui;

use crate::ids;
use super::WndRun;

impl WndRun {
	pub fn new(parent: &dyn gui::Parent) -> Self {
		let wnd        = gui::WindowModal::new_dlg(parent, ids::DLG_RUN);
		let lbl_status = gui::Label::new_dlg(&wnd, ids::LBL_STATUS);
		let pro_status = gui::ProgressBar::new_dlg(&wnd, ids::PRO_STATUS);

		let new_self = Self { wnd, lbl_status, pro_status };
		new_self.events();
		new_self
	}
}
