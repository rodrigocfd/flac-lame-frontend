use winsafe::{co, msg};

use super::WndMain;

impl WndMain {
	pub(super) fn events(&self) {
		self.wnd.on().wm_init_dialog({
			let self2 = self.clone();
			|_: msg::wm::InitDialog| -> bool {

				true
			}
		});

		self.wnd.on().wm_command_accel_menu(co::DLGID::CANCEL.into(), {
			let wnd = self.wnd.clone();
			move || {
				wnd.hwnd().PostMessage(msg::wm::Close {}).unwrap(); // close on ESC
			}
		});
	}
}
