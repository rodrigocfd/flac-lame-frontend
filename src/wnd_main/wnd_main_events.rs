use winsafe::{self as w, co, msg};

use crate::ids;
use crate::prompt;
use super::WndMain;

impl WndMain {
	pub(super) fn events(&self) {
		self.wnd.on().wm_init_dialog({
			let self2 = self.clone();
			move |_: msg::wm::InitDialog| -> bool {
				let img_list = w::HIMAGELIST::Create(16, 16, co::ILC::COLOR32, 1, 1).unwrap();
				self2.lst_files.set_image_list(co::LVSIL::SMALL, img_list);

				self2.lst_files.columns().add(&[("File", 100), ("Size", 80)]).unwrap();
				self2.lst_files.columns().set_width_to_fill(0).unwrap();

				self2.cmb_cbr.items().add(&[
					"32 kbps", "40 kbps", "48 kbps", "56 kbps",
					"64 kbps", "80 kbps", "96 kbps", "112 kbps",
					"128 kbps; default",
					"160 kbps", "192 kbps", "224 kbps", "256 kbps", "320 kbps"]).unwrap();
				self2.cmb_cbr.items().set_selected(Some(8));

				self2.cmb_vbr.items().add(&[
					"0 (~245 kbps)", "1 (~225 kbps)", "2 (~190 kbps)", "3 (~175 kbps)",
					"4 (~165 kbps); default",
					"5 (~130 kbps)", "6 (~115 kbps)", "7 (~100 kbps)", "8 (~85 kbps)", "9 (~65 kbps)"]).unwrap();
				self2.cmb_vbr.items().set_selected(Some(4));

				self2.cmb_flac_lvl.items().add(&["1", "2", "3", "4", "5", "6", "7", "8"]).unwrap();
				self2.cmb_flac_lvl.items().set_selected(Some(7));

				self2.rad_mp3_flac_wav[0].trigger_click().unwrap();
				self2.rad_cbr_vbr[0].trigger_click().unwrap();

				let mut si = w::SYSTEM_INFO::default();
				w::GetSystemInfo(&mut si);

				self2.cmb_threads.items().add(&["1", "2", "4", "6", "8", "12"]).unwrap();
				self2.cmb_threads.items().set_selected(
					Some(match si.dwNumberOfProcessors {
						2  => 1,
						4  => 2,
						6  => 3,
						8  => 4,
						12 => 5,
						_ => 0,
					}),
				);

				self2.load_ini();
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
