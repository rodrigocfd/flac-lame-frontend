use winsafe::{self as w, prelude::*, co};

use crate::util;
use super::{ini_file, WndMain};

impl WndMain {
	/// Displays the standard error message box.
	pub(super) fn msg_err(&self, caption: &str, msg: &str) -> w::AnyResult<()> {
		self.wnd.hwnd().TaskDialog(
			None,
			Some(caption),
			None,
			Some(msg),
			co::TDCBF::OK,
			w::IconRes::Error,
		).map(|_| ()).map_err(|e| e.into())
	}

	/// Updates the label of the Run button, counting the number of files to be
	/// processed.
	pub(super) fn update_run_count(&self) {
		let num_files = self.lst_files.items().count();
		if num_files == 0 {
			self.btn_run.set_text("&Run");
		} else {
			self.btn_run.set_text(&format!("&Run ({})", num_files));
		}
		self.btn_run.hwnd().EnableWindow(num_files > 0);
	}

	/// Adds the files to the main list view, retrieving their sizes.
	pub(super) fn add_files(&self, files: &[impl AsRef<str>]) -> w::AnyResult<()> {
		files.iter()
			.map(|file| file.as_ref())
			.filter(|file| self.lst_files.items().find(file).is_none()) // skip if file already added
			.try_for_each(|file| -> w::AnyResult<()> {
				let sz = w::File::open(file, w::FileAccess::ExistingReadOnly)?.size()?;
				let icon_idx = if w::path::has_extension(file, &[".mp3"]) { 0 }
					else if w::path::has_extension(file, &[".flac"]) { 1 }
					else { 2 }; // wav
				self.lst_files.items().add(&[file, &util::format_bytes(sz as _)], Some(icon_idx));
				Ok(())
			})?;

		self.lst_files.columns().get(0).set_width_to_fill();
		self.update_run_count();
		Ok(())
	}

	/// Fills the `UiSettings` object with the current state of the UI.
	#[must_use]
	pub(super) fn get_ui_settings_state(&self) -> ini_file::UiSettings {
		ini_file::UiSettings {
			target:  self.rad_mp3_flac_wav.checked_index().unwrap() as _,
			mp3enc:  self.rad_cbr_vbr.checked_index().unwrap() as _,
			cbr:     self.cmb_cbr.items().selected_index().unwrap() as _,
			vbr:     self.cmb_vbr.items().selected_index().unwrap() as _,
			flaclvl: self.cmb_flac_lvl.items().selected_index().unwrap() as _,
			delorig: self.chk_del_orig.is_checked(),
		}
	}
}
