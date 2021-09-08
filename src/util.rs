use winsafe as w;

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

pub struct Timer(i64);

impl Timer {
	pub fn start() -> Self {
		Self(w::QueryPerformanceCounter().unwrap())
	}

	pub fn now_ms(&self) -> f64 {
		let freq = w::QueryPerformanceFrequency().unwrap();
		let t1 = w::QueryPerformanceCounter().unwrap();
		((t1 - self.0) as f64 / freq as f64) * 1000.0
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

	/// Filename itself of the full path.
	pub fn get_file(path: &str) -> String {
		path[path.rfind("\\").unwrap() + 1..].to_owned()
	}

	/// Path of file, without trailing backslash.
	pub fn get_path(path: &str) -> String {
		path[..path.rfind("\\").unwrap()].to_owned()
	}

	pub fn has_extension(path: &str, extension: &str) -> bool {
		path.to_uppercase().ends_with(&extension.to_uppercase())
	}

	/// New extension must start with a dot, like `".mp3"`.
	pub fn swap_extension(path: &str, new_extension: &str) -> String {
		let mut p = path[..path.rfind('.').unwrap()].to_owned();
		p.push_str(new_extension);
		p
	}
}

pub mod prompt {
	#![allow(dead_code)]

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
		tdc.set_pszMainIcon(w::IconIdTdicon::Tdicon(ico));

		let mut title = w::WString::from_str(title);
		tdc.set_pszWindowTitle(Some(&mut title));

		let mut body = w::WString::from_str(body);
		tdc.set_pszContent(Some(&mut body));

		let (res, _) = w::TaskDialogIndirect(&mut tdc, None).unwrap();
		res
	}
}
