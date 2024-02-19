use winsafe::{prelude::*, self as w, co, msg};

use crate::ids;
use super::WndMain;

impl WndMain {
	pub(super) fn events_menu(&self) {
		let self2 = self.clone();
		self.wnd.on().wm_init_menu_popup(move |p| {
			if p.hmenu == *self2.lst_files.context_menu().unwrap() {
				let has_sel = self2.lst_files.items().selected_count() > 0;
				p.hmenu.EnableMenuItem(w::IdPos::Id(ids::MNU_REM_SEL), has_sel)?;
			}
			Ok(())
		});

		let wnd = self.wnd.clone();
		self.wnd.on().wm_command_accel_menu(co::DLGID::CANCEL.into(), move || {
			wnd.hwnd().PostMessage(msg::wm::Close {})?; // close on ESC
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_command_accel_menu(ids::MNU_OPEN, move || {
			let fileo = w::CoCreateInstance::<w::IFileOpenDialog>(
				&co::CLSID::FileOpenDialog, None, co::CLSCTX::INPROC_SERVER)?;

			fileo.SetOptions(
				fileo.GetOptions()?
					| co::FOS::FORCEFILESYSTEM
					| co::FOS::FILEMUSTEXIST
					| co::FOS::ALLOWMULTISELECT,
			)?;

			fileo.SetFileTypes(&[
				("FLAC audio files", "*.flac"),
				("WAV audio files", "*.wav"),
				("Supported audio files", "*.flac;*.wav"),
			])?;
			fileo.SetFileTypeIndex(3)?;

			if fileo.Show(self2.wnd.hwnd())? {
				self2.add_files(
					&fileo.GetResults()?
						.iter()?
						.map(|shi| shi?.GetDisplayName(co::SIGDN::FILESYSPATH))
						.collect::<w::HrResult<Vec<_>>>()?,
				)?;
			}
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_command_accel_menu(ids::MNU_REM_SEL, move || {
			self2.lst_files.items().delete_selected();
			self2.update_run_count();
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_command_accel_menu(ids::MNU_ABOUT, move || {
			let exe_name = w::HINSTANCE::NULL.GetModuleFileName()?;
			let hversion = w::HVERSIONINFO::GetFileVersionInfo(&exe_name)?;
			let version_info = hversion.version_info()?;
			let version_parts = version_info.dwFileVersion();

			self2.wnd.hwnd().TaskDialog(
				None,
				Some("About"),
				Some("FLAC/LAME front end"),
				Some(&format!(
					"Version {}.{}.{}\n\
					Writen in Rust with WinSafe library.\n\n\
					Rodrigo César de Freitas Dias © 2013-2024",
					version_parts[0], version_parts[1], version_parts[2],
				)),
				co::TDCBF::OK,
				w::IconRes::Info,
			)?;
			Ok(())
		});
	}
}
