use std::sync::{Arc, Mutex};
use winsafe::{self as w, prelude::*, co, gui};

use crate::ids;
use super::{FilesProcess, Opts, WndRun};

impl WndRun {
	pub fn new(
		parent: &impl GuiParent,
		opts: Opts,
	) -> w::AnyResult<Self>
	{
		let wnd        = gui::WindowModal::new_dlg(parent, ids::DLG_RUN);
		let lbl_status = gui::Label::new_dlg(&wnd, ids::LBL_STATUS, (gui::Horz::None, gui::Vert::None));
		let pro_status = gui::ProgressBar::new_dlg(&wnd, ids::PRO_STATUS, (gui::Horz::None, gui::Vert::None));

		let itbl = w::CoCreateInstance(
			&co::CLSID::TaskbarList, None, co::CLSCTX::INPROC_SERVER)?;

		let files_process = Arc::new(Mutex::new(FilesProcess::default()));

		let new_self = Self { wnd, lbl_status, pro_status, itbl, opts, files_process };
		new_self.events_wm();
		Ok(new_self)
	}

	pub fn show_modal(&self) -> w::AnyResult<()> {
		self.wnd.show_modal()
			.map(|_| ())
	}
}
