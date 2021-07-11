use winsafe::{self as w, co, msg, shell};

use crate::ids;
use crate::util;
use super::WndMain;

impl WndMain {
	pub(super) fn menu_events(&self) {
		self.wnd.on().wm_init_menu_popup({
			let self2 = self.clone();
			move |p: msg::wm::InitMenuPopup| {
				if p.hmenu == self2.lst_files.context_menu().unwrap() {
					let has_sel = self2.lst_files.items().selected_count() > 0;
					p.hmenu.EnableMenuItem(w::IdPos::Id(ids::MNU_REM_SEL), has_sel).unwrap();
				}
			}
		});

		self.wnd.on().wm_command_accel_menu(co::DLGID::CANCEL.into(), {
			let wnd = self.wnd.clone();
			move || {
				wnd.hwnd().PostMessage(msg::wm::Close {}).unwrap(); // close on ESC
			}
		});

		self.wnd.on().wm_command_accel_menu(ids::MNU_OPEN, {
			let self2 = self.clone();
			move || {
				let fileo = w::CoCreateInstance::<shell::IFileOpenDialog>(
					&shell::clsid::FileOpenDialog,
					None,
					co::CLSCTX::INPROC_SERVER,
				).unwrap();

				fileo.SetOptions(
					fileo.GetOptions().unwrap()
						| shell::co::FOS::FORCEFILESYSTEM
						| shell::co::FOS::FILEMUSTEXIST
						| shell::co::FOS::ALLOWMULTISELECT,
				).unwrap();

				fileo.SetFileTypes(&[
					("MP3 audio files", "*.mp3"),
					("FLAC audio files", "*.flac"),
					("WAV audio files", "*.wav"),
					("Supported audio files", "*.mp3;*.flac;*.wav"),
				]).unwrap();

				fileo.SetFileTypeIndex(4).unwrap();

				if fileo.Show(self2.wnd.hwnd()).unwrap() {
					self2.add_files(
						&fileo.GetResults().unwrap()
							.GetDisplayNames(shell::co::SIGDN::FILESYSPATH).unwrap(),
					).unwrap();
				}
			}
		});

		self.wnd.on().wm_command_accel_menu(ids::MNU_REM_SEL, {
			let self2 = self.clone();
			move || {
				self2.lst_files.items().delete_selected().unwrap();
				self2.update_run_count().unwrap();
			}
		});

		self.wnd.on().wm_command_accel_menu(ids::MNU_ABOUT, {
			let self2 = self.clone();
			move || {
				// Read version from resource.
				let exe_name = w::HINSTANCE::NULL.GetModuleFileName().unwrap();
				let mut res_buf = Vec::default();
				w::GetFileVersionInfo(&exe_name, &mut res_buf).unwrap();

				let vsffi = unsafe { w::VarQueryValue::<w::VS_FIXEDFILEINFO>(&res_buf, "\\").unwrap() };
				let ver = vsffi.dwFileVersion();

				util::prompt::info(self2.wnd.hwnd(), "About",
					&format!(
						"ID3 Padding Remover v{}.{}.{}\n\
						Writen in Rust with WinSafe library.\n\n\
						Rodrigo César de Freitas Dias © 2013-2021",
						ver[0], ver[1], ver[2]));
			}
		});
	}
}
