use std::sync::{Arc, Mutex};
use winsafe::{self as w, co, gui, shell};

use crate::ids;
use crate::util;
use super::{FilesProcess, Mp3Enc, Opts, Target, WndRun};

impl WndRun {
	pub fn new(parent: &dyn gui::Parent, opts: Opts) -> Self {
		let wnd        = gui::WindowModal::new_dlg(parent, ids::DLG_RUN);
		let lbl_status = gui::Label::new_dlg(&wnd, ids::LBL_STATUS);
		let pro_status = gui::ProgressBar::new_dlg(&wnd, ids::PRO_STATUS);

		let itbl = w::CoCreateInstance(
			&shell::clsid::TaskbarList, None, co::CLSCTX::INPROC_SERVER).unwrap();

		let files_process = Arc::new(Mutex::new(
			FilesProcess {
				idx_files_left: Vec::default(),
				num_files_done: 0
			},
		));

		let new_self = Self { wnd, lbl_status, pro_status, itbl, opts, files_process };
		new_self.events();
		new_self
	}

	pub fn show(&self) {
		self.wnd.show_modal().unwrap();
	}

	pub(super) fn process_next_file(&self, nfiles: usize) { // will always run in a parallel thread
		let idx = {
			let mut files_process = self.files_process.lock().unwrap();
			files_process.idx_files_left.remove(0) // take away first index; assumes there is at least 1
		};

		match &self.opts.target {
			Target::Mp3(enc, quality) => self.convert_to_mp3(idx, *enc, quality),
			Target::Flac(quality) => self.convert_to_flac(idx, quality),
			Target::Wav => self.convert_to_wav(idx),
		}

		let (has_more, finished_processing) = {
			let mut files_process = self.files_process.lock().unwrap();
			files_process.num_files_done += 1;

			self.wnd.run_ui_thread(|| { // progress, update UI
				self.lbl_status.set_text(
					&format!("{} of {} file(s) finished...",
						files_process.num_files_done, nfiles)).unwrap();
				self.itbl.SetProgressValue(
					self.wnd.hwnd().GetAncestor(co::GA::ROOTOWNER).unwrap(),
					files_process.num_files_done as _, nfiles as _).unwrap();
				self.pro_status.set_position(files_process.num_files_done as _);
			});

			(!files_process.idx_files_left.is_empty(), files_process.num_files_done == nfiles)
		};

		if has_more {
			self.process_next_file(nfiles);
		} else if finished_processing {
			self.wnd.run_ui_thread(|| { // finished, update UI
				self.itbl.SetProgressValue(self.wnd.hwnd(), nfiles as _, nfiles as _).unwrap();
				self.pro_status.set_position(nfiles as _);
			});

			w::Sleep(500); // so we can briefly see the 100%

			self.wnd.run_ui_thread(|| { // cleanup, update UI
				self.itbl.SetProgressState(
					self.wnd.hwnd().GetAncestor(co::GA::ROOTOWNER).unwrap(),
					shell::co::TBPF::NOPROGRESS).unwrap();
				self.wnd.hwnd().EndDialog(0).unwrap();
			});
		}
	}

	pub(super) fn convert_to_mp3(&self, idx: usize, enc: Mp3Enc, quality: &str) {
		let file_path = &self.opts.files[idx];
		if util::path::has_extension(file_path, ".flac") {

		} else if util::path::has_extension(file_path, ".wav") {

		}

		winsafe::Sleep(2000);
		println!("mp3 {} {} {}", idx, quality, winsafe::GetCurrentThreadId());
	}

	pub(super) fn convert_to_flac(&self, idx: usize, quality: &str) {

	}

	pub(super) fn convert_to_wav(&self, idx: usize) {

	}
}
