use winsafe::{self as w, prelude::*, co};

use crate::util;
use super::WndMain;

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

	/// Returns the paths of LAME and FLAC tools, read from the local INI file.
	pub(super) fn read_tool_paths() -> w::AnyResult<(String, String)> {
		let ini_file_path = format!("{}\\flac-lame-frontend.ini", w::path::exe_path()?);
		let lame_path = match w::GetPrivateProfileString("Tools", "lame", &ini_file_path)? {
			Some(lame_path) => lame_path,
			None => {
				return Err(format!("LAME path not found at:\n{}", ini_file_path).into());
			},
		};
		let flac_path = match w::GetPrivateProfileString("Tools", "flac", &ini_file_path)? {
			Some(lame_path) => lame_path,
			None => {
				return Err(format!("FLAC path not found at:\n{}", ini_file_path).into());
			},
		};

		if !w::path::exists(&lame_path) {
			Err(format!("LAME tool not found at:\n{}", lame_path).into())
		} else if !w::path::exists(&flac_path) {
			Err(format!("FLAC tool not found at:\n{}", flac_path).into())
		} else {
			Ok((lame_path, flac_path))
		}
	}
}
