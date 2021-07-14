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
			Target::Mp3(enc, quality) => self.convert_to_mp3(&self.opts.files[idx], *enc, quality),
			Target::Flac(quality) => self.convert_to_flac(&self.opts.files[idx], quality),
			Target::Wav => self.convert_to_wav(&self.opts.files[idx]),
		}

		self.opts.dest_folder.as_ref().map(|dest_folder: &String| { // move original file if due
			let converted_file = util::path::swap_extension(&self.opts.files[idx],
				match &self.opts.target {
					Target::Mp3(_, _) => ".mp3",
					Target::Flac(_) => ".flac",
					Target::Wav => ".wav",
				});
			w::MoveFile(&converted_file,
				&format!("{}\\{}", dest_folder, util::path::get_file(&converted_file)),
			).unwrap();
		});

		if self.opts.del_orig { // delete original file if due
			w::DeleteFile(&self.opts.files[idx]).unwrap();
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
				self.itbl.SetProgressState(
					self.wnd.hwnd().GetAncestor(co::GA::ROOTOWNER).unwrap(),
					shell::co::TBPF::NOPROGRESS).unwrap();
				self.wnd.hwnd().EndDialog(0).unwrap();
			});
		}
	}

	pub(super) fn convert_to_mp3(&self, src_path: &str, enc: Mp3Enc, quality: &str) {
		let wav_to_mp3 = |the_src: &str| {
			Self::raw_run_cmd(
				&format!("\"{}\" -{}{} --noreplaygain \"{}\"", // convert to MP3 with LAME
					self.opts.lame_path,
					match enc {
						Mp3Enc::Cbr => "b",
						Mp3Enc::Vbr => "V",
					},
					quality, the_src),
			);
		};

		if util::path::has_extension(src_path, ".flac") {
			Self::raw_run_cmd(
				&format!("\"{}\" -d \"{}\"", // intermediary convert to WAV with FLAC
					self.opts.flac_path, src_path),
			);
			let intermediary_wav = util::path::swap_extension(src_path, ".wav");
			wav_to_mp3(&intermediary_wav);
			w::DeleteFile(&intermediary_wav).unwrap();
		} else if util::path::has_extension(src_path, ".wav") {
			wav_to_mp3(src_path);
		}
	}

	pub(super) fn convert_to_flac(&self, src_path: &str, quality: &str) {
		let wav_to_flac = |the_src: &str| {
			Self::raw_run_cmd(
				&format!("\"{}\" -{} -V --no-seektable \"{}\"",
					self.opts.flac_path, quality, the_src),
			);
			println!("{}", &format!("\"{}\" -{} -V --no-seektable \"{}\"",
			self.opts.flac_path, quality, the_src));
		};

		if util::path::has_extension(src_path, ".flac") {
			Self::raw_run_cmd(
				&format!("\"{}\" -d \"{}\"", // intermediary convert to WAV with FLAC
					self.opts.flac_path, src_path),
			);
			let intermediary_wav = util::path::swap_extension(src_path, ".wav");
			wav_to_flac(&intermediary_wav);
			w::DeleteFile(&intermediary_wav).unwrap();
		} else if util::path::has_extension(src_path, ".wav") {
			wav_to_flac(src_path);
		}
	}

	pub(super) fn convert_to_wav(&self, src_path: &str) {
		if util::path::has_extension(src_path, ".flac") { // if wav already, nothing is done
			Self::raw_run_cmd(
				&format!("\"{}\" -d \"{}\"",
					self.opts.flac_path, src_path),
			);
		}
	}

	fn raw_run_cmd(cmd_line: &str) -> u32 {
		let mut sa = w::SECURITY_ATTRIBUTES::default();
		sa.set_bInheritHandle(true);

		let mut si = w::STARTUPINFO::default();
		si.dwFlags = co::STARTF::USESHOWWINDOW;
		si.set_wShowWindow(co::SW::SHOW);

		let pi = w::HPROCESS::CreateProcess(None, Some(cmd_line), Some(&mut sa),
			None, false, co::CREATE::NONE, None, None, &mut si).unwrap();
		defer! { pi.hThread.CloseHandle().unwrap(); }
		defer! { pi.hProcess.CloseHandle().unwrap(); }

		pi.hProcess.WaitForSingleObject(None).unwrap();
		pi.hProcess.GetExitCodeProcess().unwrap()
	}
}
