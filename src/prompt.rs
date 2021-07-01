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
