use winsafe::{self as w, prelude::*, co, msg};

use crate::{ids, util, wnd_run};
use super::{ini_file, WndMain};

impl WndMain {
	pub(super) fn events_wm(&self) {
		let self2 = self.clone();
		self.wnd.on().wm_init_dialog(move |_| {
			self2.init_dialog()
		});

		let self2 = self.clone();
		self.wnd.on().wm_close(move || {
			let ui_settings = self2.get_ui_settings_state();
			if let Err(e) = ini_file::serialize_ui_settings(&ui_settings) {
				self2.msg_err("Error saving INI", &e.to_string()).ok();
			}
			self2.wnd.hwnd().DestroyWindow()?;
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_size(move |p| {
			if p.request != co::SIZE_R::MINIMIZED {
				self2.lst_files.columns().get(0).set_width_to_fill();
			}
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_get_min_max_info(move |p| {
			let min_sz = self2.min_sz.get();
			p.info.ptMinTrackSize = w::POINT::new(min_sz.cx, min_sz.cy); // limit min size
			Ok(())
		});

		let self2 = self.clone();
		self.wnd.on().wm_drop_files(move |mut p| {
			let dropped_files = p.hdrop.DragQueryFile()?.collect::<w::SysResult<Vec<_>>>()?;
			let mut valid_files = Vec::<String>::with_capacity(dropped_files.len());

			for file in dropped_files.iter() {
				if w::path::is_directory(file) {
					for ext in ["*.flac", "*.wav"] {
						for sub_file in w::path::dir_list(file, Some(ext)) {
							valid_files.push(sub_file?); // search only 1 level below
						}
					}
				} else if w::path::has_extension(file, &[".flac", ".wav"]) {
					valid_files.push(file.clone());
				}
			}

			if valid_files.is_empty() {
				self2.msg_err(
					"No files added",
					&format!("{} file(s) have been dropped, but none of them was WAV or FLAC.",
						dropped_files.len()),
				)?;
			} else {
				self2.add_files(&valid_files).unwrap();
			}

			Ok(())
		});

		let self2 = self.clone();
		self.lst_files.on().lvn_key_down(move |p| {
			if p.wVKey == co::VK::DELETE { // delete item on DEL
				self2.wnd.hwnd().SendMessage(msg::wm::Command {
					event: w::AccelMenuCtrl::Menu(ids::MNU_REM_SEL),
				});
			}
			Ok(())
		});

		let self2 = self.clone();
		self.rad_mp3_flac_wav.on().bn_clicked(move || {
			let checked_idx = self2.rad_mp3_flac_wav.checked_index().unwrap();

			self2.rad_cbr_vbr.iter().for_each(|radio| {
				radio.hwnd().EnableWindow(checked_idx == 0);
				if radio.is_selected() {
					radio.emulate_click();
				}
			});

			self2.lbl_flac_lvl.hwnd().EnableWindow(checked_idx == 1);
			self2.cmb_flac_lvl.hwnd().EnableWindow(checked_idx == 1);
			Ok(())
		});

		let self2 = self.clone();
		self.rad_cbr_vbr.on().bn_clicked(move || {
			let is_mp3 = self2.rad_mp3_flac_wav.checked_index().unwrap() == 0;
			let checked_idx = self2.rad_cbr_vbr.checked_index().unwrap();

			self2.cmb_cbr.hwnd().EnableWindow(checked_idx == 0 && is_mp3);
			self2.cmb_vbr.hwnd().EnableWindow(checked_idx == 1 && is_mp3);
			Ok(())
		});

		let self2 = self.clone();
		self.btn_dest.on().bn_clicked(move || {
			let foldero = w::CoCreateInstance::<w::IFileOpenDialog>(
				&co::CLSID::FileOpenDialog, None, co::CLSCTX::INPROC_SERVER)?;

			foldero.SetOptions(
				foldero.GetOptions()?
					| co::FOS::FORCEFILESYSTEM
					| co::FOS::FILEMUSTEXIST
					| co::FOS::PICKFOLDERS,
			)?;

			if foldero.Show(self2.wnd.hwnd())? {
				self2.txt_dest.set_text(
					&foldero.GetResult()?
						.GetDisplayName(co::SIGDN::FILESYSPATH)?,
				);
			}
			Ok(())
		});

		let self2 = self.clone();
		self.btn_run.on().bn_clicked(move || {
			let new_dest = self2.txt_dest.text().trim().to_owned();
			if !new_dest.is_empty() && !w::path::exists(&new_dest) {
				self2.msg_err(
					"Invalid directory",
					&format!("Target directory does not exist:\n{}", new_dest),
				)?;
				self2.txt_dest.set_selection(0, -1);
				self2.txt_dest.hwnd().SetFocus();
				return Ok(());
			}

			let idx_type = self2.rad_mp3_flac_wav.checked_index().unwrap();
			let target = match idx_type {
				0 => { // mp3
					if self2.rad_cbr_vbr.checked_index().unwrap() == 0 { // mp3/cbr
						let txt_quality = self2.cmb_cbr.items().selected_text().unwrap();
						let idx_space = txt_quality.find(' ').unwrap_or_default();
						wnd_run::Target::Mp3(wnd_run::Mp3Enc::Cbr, txt_quality[..idx_space].to_owned())
					} else { // mp3/vbr
						let txt_quality = self2.cmb_vbr.items().selected_text().unwrap();
						let idx_space = txt_quality.find(' ').unwrap_or_default();
						wnd_run::Target::Mp3(wnd_run::Mp3Enc::Vbr, txt_quality[..idx_space].to_owned())
					}
				},
				1 => { // flac
					let txt_quality = self2.cmb_flac_lvl.items().selected_text().unwrap();
					wnd_run::Target::Flac(txt_quality)
				},
				2 => { // wav
					let first_wav = self2.lst_files.items()
						.iter()
						.map(|item| item.text(0))
						.find(|file_path| w::path::has_extension(&file_path, &[".wav"]));
					if let Some(wav_file) = first_wav {
						self2.msg_err(
							"Bad conversion",
							&format!("Cannot convert WAV to WAV:\n{}", &wav_file),
						)?;
						return Ok(()); // no further processing is done
					}
					wnd_run::Target::Wav
				},
				_ => {
					return Err(format!("Invalid type index: {}.", idx_type).into())
				},
			};

			let dest_folder = w::path::rtrim_backslash(&self2.txt_dest.text()).to_owned();

			let (lame_path, flac_path) = match ini_file::read_tool_paths() {
				Ok((flac_path, lame_path)) => (flac_path, lame_path),
				Err(e) => {
					self2.msg_err("Tool not found", &e.to_string())?;
					return Ok(());
				},
			};

			let run_opts = wnd_run::Opts {
				lame_path,
				flac_path,
				files: self2.lst_files.items()
					.iter()
					.map(|item| item.text(0))
					.collect::<Vec<_>>(),
				dest_folder: if dest_folder.is_empty() { None } else { Some(dest_folder) },
				target,
				del_orig: self2.chk_del_orig.is_checked(),
				num_threads: self2.cmb_threads.items().selected_text().unwrap().parse().unwrap(),
			};

			let t0 = util::Timer::start();

			let runner = wnd_run::WndRun::new(&self2.wnd, run_opts)?;
			runner.show_modal()?;

			self2.wnd.hwnd().TaskDialog(
				None,
				Some("Process finished"),
				None,
				Some(&format!("{} file(s) processed in {:.3} secs.",
					self2.lst_files.items().count(), t0.now_ms() / 1000.0)),
				co::TDCBF::OK,
				w::IconRes::Info,
			)?;
			Ok(())
		});
	}
}
