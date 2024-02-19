use winsafe::{self as w, prelude::*, co};

use super::{Mp3Enc, Target, WndRun};

impl WndRun {
	/// Will always run in a parallel thread.
	pub(super) fn process_next_file(&self, nfiles: usize) -> w::AnyResult<()> {
		let idx = {
			let mut files_process = self.files_process.lock().unwrap();
			files_process.idx_files_left.remove(0) // take away first index; assumes there is at least 1
		};

		match &self.opts.target {
			Target::Mp3(enc, quality) => self.convert_to_mp3(&self.opts.files[idx], *enc, quality)?,
			Target::Flac(quality) => self.convert_to_flac(&self.opts.files[idx], quality)?,
			Target::Wav => self.run_convert_to_wav(&self.opts.files[idx])?,
		}

		self.opts.dest_folder.as_ref().map(|dest_folder| { // move original file if due
			let converted_file = w::path::replace_extension(
				&self.opts.files[idx],
				match &self.opts.target {
					Target::Mp3(_, _) => ".mp3",
					Target::Flac(_) => ".flac",
					Target::Wav => ".wav",
				});
			w::MoveFile(&converted_file, &format!(
				"{}\\{}",
				dest_folder,
				w::path::get_file_name(&converted_file).unwrap(),
			)).unwrap();
		});

		if self.opts.del_orig { // delete original file if due
			w::DeleteFile(&self.opts.files[idx]).unwrap();
		}

		let (has_more, finished_processing) = {
			let mut files_process = self.files_process.lock().unwrap();
			files_process.num_files_done += 1;

			let self2 = self.clone();
			let num_files_done = files_process.num_files_done;
			self.wnd.run_ui_thread(move || { // progress, update UI
				self2.lbl_status.set_text(
					&format!("{} of {} file(s) finished...",
						num_files_done, nfiles),
				);
				self2.itbl.SetProgressValue(
					&self2.wnd.hwnd().GetAncestor(co::GA::ROOTOWNER).unwrap(),
					num_files_done as _,
					nfiles as _,
				)?;
				self2.pro_status.set_position(num_files_done as _);
				Ok(())
			});

			(!files_process.idx_files_left.is_empty(), files_process.num_files_done == nfiles)
		};

		if has_more {
			self.process_next_file(nfiles)?;
		} else if finished_processing {
			let self2 = self.clone();
			self.wnd.run_ui_thread(move || { // finished, update UI
				self2.itbl.SetProgressState(
					&self2.wnd.hwnd().GetAncestor(co::GA::ROOTOWNER).unwrap(),
					co::TBPF::NOPROGRESS,
				)?;
				self2.wnd.hwnd().EndDialog(0)?;
				Ok(())
			});
		}

		Ok(())
	}

	fn convert_to_mp3(&self, src_path: &str, enc: Mp3Enc, quality: &str) -> w::AnyResult<()> {
		let run_wav_to_mp3 = |the_src: &str| -> w::AnyResult<()> {
			Self::raw_run_cmd_sync(
				&format!("\"{}\" -{}{} --noreplaygain \"{}\"", // convert to MP3 with LAME
					self.opts.lame_path,
					match enc {
						Mp3Enc::Cbr => "b",
						Mp3Enc::Vbr => "V",
					},
					quality, the_src),
			)?;
			Ok(())
		};

		if w::path::has_extension(src_path, &[".flac"]) {
			Self::raw_run_cmd_sync(
				&format!("\"{}\" -d \"{}\"", // intermediary convert to WAV with FLAC
					self.opts.flac_path, src_path),
			)?;
			let intermediary_wav = w::path::replace_extension(src_path, ".wav");
			run_wav_to_mp3(&intermediary_wav)?;
			w::DeleteFile(&intermediary_wav).unwrap();
		} else if w::path::has_extension(src_path, &[".wav"]) {
			run_wav_to_mp3(src_path)?;
		}

		Ok(())
	}

	fn convert_to_flac(&self, src_path: &str, quality: &str) -> w::AnyResult<()> {
		let run_wav_to_flac = |the_src: &str| -> w::AnyResult<()> {
			Self::raw_run_cmd_sync(
				&format!("\"{}\" -{} -V --no-seektable \"{}\"",
					self.opts.flac_path, quality, the_src),
			)?;
			println!("{}", &format!("\"{}\" -{} -V --no-seektable \"{}\"",
			self.opts.flac_path, quality, the_src));
			Ok(())
		};

		if w::path::has_extension(src_path, &[".flac"]) {
			Self::raw_run_cmd_sync(
				&format!("\"{}\" -d \"{}\"", // intermediary convert to WAV with FLAC
					self.opts.flac_path, src_path),
			)?;
			let intermediary_wav = w::path::replace_extension(src_path, ".wav");
			run_wav_to_flac(&intermediary_wav)?;
			w::DeleteFile(&intermediary_wav).unwrap();
		} else if w::path::has_extension(src_path, &[".wav"]) {
			run_wav_to_flac(src_path)?;
		}

		Ok(())
	}

	fn run_convert_to_wav(&self, src_path: &str) -> w::AnyResult<()> {
		if w::path::has_extension(src_path, &[".flac"]) { // if wav already, nothing is done
			Self::raw_run_cmd_sync(
				&format!("\"{}\" -d \"{}\"",
					self.opts.flac_path, src_path),
			)?;
		}

		Ok(())
	}

	/// Runs the given command line in a new process, synchronously.
	fn raw_run_cmd_sync(cmd_line: &str) -> w::AnyResult<u32> {
		let mut sa = w::SECURITY_ATTRIBUTES::default();
		sa.set_bInheritHandle(true);

		let mut si = w::STARTUPINFO::default();
		si.dwFlags = co::STARTF::USESHOWWINDOW;
		si.set_wShowWindow(co::SW::SHOW);

		let pi = w::HPROCESS::CreateProcess(
			None, Some(cmd_line), Some(&mut sa), None, false,
			co::CREATE::NoValue, None, None, &mut si)?;

		pi.hProcess.WaitForSingleObject(None)?;
		pi.hProcess.GetExitCodeProcess()
			.map_err(|e| e.into())
	}
}
