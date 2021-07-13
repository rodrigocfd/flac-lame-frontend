pub fn format_bytes(num_bytes: usize) -> String {
	if num_bytes < 1024 {
		format!("{} bytes", num_bytes)
	} else if num_bytes < 1024 * 1024 {
		format!("{:.2} KB", (num_bytes as f64) / 1024.0)
	} else if num_bytes < 1024 * 1024 * 1024 {
		format!("{:.2} MB", (num_bytes as f64) / 1024.0 / 1024.0)
	} else if num_bytes < 1024 * 1024 * 1024 * 1024 {
		format!("{:.2} GB", (num_bytes as f64) / 1024.0 / 1024.0 / 1024.0)
	} else if num_bytes < 1024 * 1024 * 1024 * 1024 * 1024 {
		format!("{:.2} TB", (num_bytes as f64) / 1024.0 / 1024.0 / 1024.0 / 1024.0)
	} else {
		format!("{:.2} PB", (num_bytes as f64) / 1024.0 / 1024.0 / 1024.0 / 1024.0 / 1024.0)
	}
}

pub mod path {
	use std::error::Error;
	use winsafe::{self as w, co};

	pub fn is_dir(path: &str) -> Result<bool, Box<dyn Error>> {
		Ok(w::GetFileAttributes(path)?.has(co::FILE_ATTRIBUTE::DIRECTORY))
	}

	pub fn exists(path: &str) -> bool {
		w::GetFileAttributes(path).is_ok()
	}

	/// Path of file, without trailing backslash.
	pub fn get_path(path: &str) -> String {
		path[..path.rfind("\\").unwrap()].to_owned()
	}

	pub fn has_extension(path: &str, extension: &str) -> bool {
		path.to_uppercase().ends_with(&extension.to_uppercase())
	}
}

pub mod prompt {
	use winsafe::{self as w, co};

	pub fn err(hwnd: w::HWND, title: &str, body: &str) {
		base(hwnd, title, body, co::TDCBF::OK, co::TD_ICON::ERROR);
	}

	pub fn info(hwnd: w::HWND, title: &str, body: &str) {
		base(hwnd, title, body, co::TDCBF::OK, co::TD_ICON::INFORMATION);
	}

	pub fn ok_cancel(hwnd: w::HWND, title: &str, body: &str) -> co::DLGID {
		base(hwnd, title, body, co::TDCBF::OK | co::TDCBF::CANCEL, co::TD_ICON::WARNING)
	}

	fn base(hwnd: w::HWND, title: &str, body: &str,
		btns: co::TDCBF, ico: co::TD_ICON) -> co::DLGID
	{
		let mut tdc = w::TASKDIALOGCONFIG::default();
		tdc.hwndParent = hwnd;
		tdc.dwFlags = co::TDF::ALLOW_DIALOG_CANCELLATION;
		tdc.dwCommonButtons = btns;
		tdc.set_hMainIcon(w::HiconIdTdicon::Tdicon(ico));

		let mut title = w::WString::from_str(title);
		tdc.set_pszWindowTitle(Some(&mut title));

		let mut body = w::WString::from_str(body);
		tdc.set_pszContent(Some(&mut body));

		let (res, _) = w::TaskDialogIndirect(&mut tdc, None).unwrap();
		res
	}
}
