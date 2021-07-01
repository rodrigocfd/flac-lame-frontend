use winsafe::{self as w, gui};

use crate::ids;
use super::WndMain;

impl WndMain {
	pub fn new() -> Self {
		let wnd = gui::WindowMain::new_dlg(ids::DLG_MAIN, Some(ids::ICO_MAIN), Some(ids::ACC_MAIN));

		let new_self = Self { wnd };
		new_self.events();
		new_self
	}

	pub fn run(&self) -> w::WinResult<()> {
		self.wnd.run_main(None)
	}
}
