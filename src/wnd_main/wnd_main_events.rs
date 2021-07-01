use winsafe::{self as w, co, msg};

use crate::ids;
use crate::prompt;
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

		self.wnd.on().wm_command_accel_menu(ids::MNU_OPEN, {
			move || {

			}
		});

		self.wnd.on().wm_command_accel_menu(ids::MNU_REM_SEL, {
			move || {

			}
		});

		self.wnd.on().wm_command_accel_menu(ids::MNU_ABOUT, {
			let self2 = self.clone();
			move || {
				// Read version from resource.
				let exe_name = w::HINSTANCE::NULL.GetModuleFileName().unwrap();
				let mut res_buf = Vec::default();
				w::GetFileVersionInfo(&exe_name, &mut res_buf).unwrap();

				let fis = w::VarQueryValue(&res_buf, "\\").unwrap();
				let fi: &w::VS_FIXEDFILEINFO = unsafe { &*(fis.as_ptr() as *const w::VS_FIXEDFILEINFO) };
				let ver = fi.dwFileVersion();

				prompt::info(self2.wnd.hwnd(), "About",
					&format!(
						"ID3 Padding Remover v{}.{}.{}\n\
						Writen in Rust with WinSafe library.\n\n\
						Rodrigo César de Freitas Dias © 2013-2021",
						ver[0], ver[1], ver[2]));
			}
		});
	}
}
