use std::sync::{Arc, Mutex};
use winsafe::gui;

use crate::ids;
use super::{Mp3Enc, Opts, Target, WndRun};

impl WndRun {
	pub fn new(parent: &dyn gui::Parent, opts: Opts) -> Self {
		let wnd        = gui::WindowModal::new_dlg(parent, ids::DLG_RUN);
		let lbl_status = gui::Label::new_dlg(&wnd, ids::LBL_STATUS);
		let pro_status = gui::ProgressBar::new_dlg(&wnd, ids::PRO_STATUS);
		let files_left = Arc::new(Mutex::new(Vec::default()));
		let files_done = Arc::new(Mutex::new(0));

		let new_self = Self { wnd, lbl_status, pro_status, opts, files_left, files_done };
		new_self.events();
		new_self
	}

	pub fn show(&self) {
		self.wnd.show_modal().unwrap();
	}

	pub(super) fn process_next_file(&self, nfiles: usize) {
		let idx = {
			let mut files_left = self.files_left.lock().unwrap();
			files_left.remove(0) // remove first index; assumes there is at least 1
		};

		match &self.opts.target {
			Target::Mp3(enc, quality) => self.convert_to_mp3(idx, *enc, quality),
			Target::Flac(quality) => self.convert_to_flac(idx, quality),
			Target::Wav => self.convert_to_wav(idx),
		}

		let (has_more, finished_processing) = {
			let mut files_done = self.files_done.lock().unwrap();
			*files_done += 1;

			self.wnd.run_ui_thread(|| {
				self.lbl_status.set_text(
					&format!("{} of {} files finished...", *files_done, nfiles)).unwrap();
				self.pro_status.set_position(*files_done as _);
			});

			(!self.files_left.lock().unwrap().is_empty(), *files_done == nfiles)
		};

		if has_more {
			self.process_next_file(nfiles);
		} else if finished_processing {
			println!("DONE");
		}
	}

	pub(super) fn convert_to_mp3(&self, idx: usize, enc: Mp3Enc, quality: &str) {
		winsafe::Sleep(2000);
		println!("mp3 {} {} {}", idx, quality, winsafe::GetCurrentThreadId());
	}

	pub(super) fn convert_to_flac(&self, idx: usize, quality: &str) {

	}

	pub(super) fn convert_to_wav(&self, idx: usize) {

	}
}
