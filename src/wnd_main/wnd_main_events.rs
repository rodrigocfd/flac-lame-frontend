use winsafe::{self as w, co, gui, msg, shell};

use crate::ids;
use crate::util;
use crate::wnd_run;
use super::WndMain;

impl WndMain {
	pub(super) fn events(&self) {
		self.wnd.on().wm_init_dialog({
			let self2 = self.clone();
			move |_: msg::wm::InitDialog| -> bool {
				// Since it doesn't have LVS_SHAREIMAGELISTS style, the image
				// list will be automatically deleted by the list view.
				let himg = w::HIMAGELIST::Create(16, 16, co::ILC::COLOR32, 1, 1).unwrap();
				himg.AddIconFromShell(&["mp3", "flac", "wav"]).unwrap();
				self2.lst_files.set_image_list(co::LVSIL::SMALL, himg);

				self2.lst_files.columns().add(&[("File", 100), ("Size", 70)]).unwrap();
				self2.lst_files.columns().set_width_to_fill(0).unwrap();

				self2.cmb_cbr.items().add(&[
					"32 kbps", "40 kbps", "48 kbps", "56 kbps",
					"64 kbps", "80 kbps", "96 kbps", "112 kbps",
					"128 kbps; default",
					"160 kbps", "192 kbps", "224 kbps", "256 kbps", "320 kbps"]).unwrap();
				self2.cmb_cbr.items().set_selected(Some(8));

				self2.cmb_vbr.items().add(&[
					"0 (~245 kbps)", "1 (~225 kbps)", "2 (~190 kbps)", "3 (~175 kbps)",
					"4 (~165 kbps); default",
					"5 (~130 kbps)", "6 (~115 kbps)", "7 (~100 kbps)", "8 (~85 kbps)", "9 (~65 kbps)"]).unwrap();
				self2.cmb_vbr.items().set_selected(Some(4));

				self2.cmb_flac_lvl.items().add(&["1", "2", "3", "4", "5", "6", "7", "8"]).unwrap();
				self2.cmb_flac_lvl.items().set_selected(Some(7));

				self2.rad_mp3_flac_wav[0].trigger_click();
				self2.rad_cbr_vbr[1].trigger_click();

				let mut si = w::SYSTEM_INFO::default();
				w::GetSystemInfo(&mut si);

				self2.cmb_threads.items().add(&["1", "2", "4", "6", "8", "12"]).unwrap();
				self2.cmb_threads.items().set_selected(
					Some(match si.dwNumberOfProcessors {
						2  => 1,
						4  => 2,
						6  => 3,
						8  => 4,
						12 => 5,
						_ => 0,
					}),
				);

				let rc = self2.wnd.hwnd().GetWindowRect().unwrap();
				self2.min_sz.replace(w::SIZE::new(rc.right - rc.left, rc.bottom - rc.top - 200));

				true
			}
		});

		self.wnd.on().wm_size({
			let lst_files = self.lst_files.clone();
			move |p: msg::wm::Size| {
				if p.request != co::SIZE_R::MINIMIZED {
					lst_files.columns().set_width_to_fill(0).unwrap();
				}
			}
		});

		self.wnd.on().wm_get_min_max_info({
			let self2 = self.clone();
			move |p: msg::wm::GetMinMaxInfo| {
				let min_sz = self2.min_sz.get();
				p.info.ptMinTrackSize = w::POINT::new(min_sz.cx, min_sz.cy); // limit min size
			}
		});

		self.wnd.on().wm_drop_files({
			let self2 = self.clone();
			move |p: msg::wm::DropFiles| {
				let dropped_files = p.hdrop.DragQueryFiles().unwrap();
				let mut all_files = Vec::with_capacity(dropped_files.len());

				[".flac", ".wav"].iter().for_each(|ext: &&str| {
					dropped_files.iter().for_each(|file: &String| {
						let mut file = file.clone();
						file.reserve(file.len() + 10); // arbitrary

						if util::path::is_dir(&file).unwrap() {
							if !file.ends_with('\\') {
								file.push('\\');
							}
							file.push('*');
							file.push_str(*ext); // *.flac

							for sub_file in w::HFINDFILE::ListAll(&file).unwrap() { // just search 1 level below
								if sub_file.to_lowercase().ends_with(*ext) {
									all_files.push(sub_file);
								}
							}
						} else if file.to_lowercase().ends_with(*ext) {
							all_files.push(file);
						}
					});
				});

				if all_files.is_empty() {
					util::prompt::err(self2.wnd.hwnd(), "No files added",
						&format!("{} file(s) have been dropped, but none of them was WAV or FLAC.",
							dropped_files.len()));
				} else {
					self2.add_files(&all_files).unwrap();
				}
			}
		});

		self.lst_files.on().lvn_key_down({
			let self2 = self.clone();
			move |p: &w::NMLVKEYDOWN| {
				if p.wVKey == co::VK::DELETE { // delete item on DEL
					self2.wnd.hwnd().SendMessage(msg::wm::Command {
						event: w::AccelMenuCtrl::Menu(ids::MNU_REM_SEL),
					});
				}
			}
		});

		self.rad_mp3_flac_wav.on().bn_clicked({
			let self2 = self.clone();
			move || {
				let checked_idx = self2.rad_mp3_flac_wav.checked_index().unwrap();

				self2.rad_cbr_vbr.iter().for_each(|radio: &gui::RadioButton| {
					radio.hwnd().EnableWindow(checked_idx == 0);
					if radio.is_checked() {
						radio.trigger_click();
					}
				});

				self2.lbl_flac_lvl.hwnd().EnableWindow(checked_idx == 1);
				self2.cmb_flac_lvl.hwnd().EnableWindow(checked_idx == 1);
			}
		});

		self.rad_cbr_vbr.on().bn_clicked({
			let self2 = self.clone();
			move || {
				let is_mp3 = self2.rad_mp3_flac_wav.checked_index().unwrap() == 0;
				let checked_idx = self2.rad_cbr_vbr.checked_index().unwrap();

				self2.cmb_cbr.hwnd().EnableWindow(checked_idx == 0 && is_mp3);
				self2.cmb_vbr.hwnd().EnableWindow(checked_idx == 1 && is_mp3);
			}
		});

		self.btn_dest.on().bn_clicked({
			let self2 = self.clone();
			move || {
				let foldero: shell::IFileOpenDialog = w::CoCreateInstance(
					&shell::clsid::FileOpenDialog,
					None,
					co::CLSCTX::INPROC_SERVER,
				).unwrap();

				foldero.SetOptions(
					foldero.GetOptions().unwrap()
						| shell::co::FOS::FORCEFILESYSTEM
						| shell::co::FOS::FILEMUSTEXIST
						| shell::co::FOS::PICKFOLDERS,
				).unwrap();

				if foldero.Show(self2.wnd.hwnd()).unwrap() {
					self2.txt_dest.set_text(
						&foldero.GetResult().unwrap()
							.GetDisplayName(shell::co::SIGDN::FILESYSPATH).unwrap(),
					).unwrap();
				}
			}
		});

		self.btn_run.on().bn_clicked({
			let self2 = self.clone();
			move || {
				let new_dest = self2.txt_dest.text().unwrap().trim().to_owned();
				if !new_dest.is_empty() && !util::path::exists(&new_dest) {
					util::prompt::err(self2.wnd.hwnd(), "Invalid directory",
						&format!("Target directory does not exist:\n{}", new_dest));
					self2.txt_dest.set_selection(Some(0), None);
					self2.txt_dest.hwnd().SetFocus();
					return;
				}

				let idx_type = self2.rad_mp3_flac_wav.checked_index().unwrap();

				for file in self2.lst_files.columns().all_texts(0).iter() {
					if idx_type == 2 && file.ends_with(".wav") {
						util::prompt::err(self2.wnd.hwnd(), "Bad conversion",
							&format!("Cannot convert WAV to WAV:\n{}", file));
						return;
					}
				}

				let target = if idx_type == 0 { // mp3
					if self2.rad_cbr_vbr.checked_index().unwrap() == 0 { // mp3/cbr
						let txt_quality = self2.cmb_cbr.items().selected_text().unwrap();
						let idx_space = txt_quality.find(' ').unwrap_or_default();
						wnd_run::Target::Mp3(wnd_run::Mp3Enc::Cbr, txt_quality[..idx_space].to_owned())
					} else { // mp3/vbr
						let txt_quality = self2.cmb_vbr.items().selected_text().unwrap();
						let idx_space = txt_quality.find(' ').unwrap_or_default();
						wnd_run::Target::Mp3(wnd_run::Mp3Enc::Vbr, txt_quality[..idx_space].to_owned())
					}
				} else if idx_type == 1 { // flac
					let txt_quality = self2.cmb_flac_lvl.items().selected_text().unwrap();
					wnd_run::Target::Flac(txt_quality)
				} else { // wav
					wnd_run::Target::Wav
				};

				let dest_folder = { // without trailing backslash
					let mut dest_folder = self2.txt_dest.text().unwrap();
					if dest_folder.ends_with('\\') {
						dest_folder.pop();
					}
					dest_folder
				};

				let ini_file = w::Ini::parse_from_file(
					&format!("{}\\flac-lame-frontend.ini", Self::real_exe_path())).unwrap();
				let lame_path = ini_file.value("Tools", "lame").unwrap().to_owned();
				let flac_path = ini_file.value("Tools", "flac").unwrap().to_owned();
				if !util::path::exists(&lame_path) {
					util::prompt::err(self2.wnd.hwnd(), "LAME not found",
						&format!("LAME not found at:\n{}", lame_path));
					return;
				} else if !util::path::exists(&flac_path) {
					util::prompt::err(self2.wnd.hwnd(), "FLAC not found",
						&format!("FLAC not found at:\n{}", lame_path));
					return;
				}

				let run_opts = wnd_run::Opts {
					lame_path,
					flac_path,
					files: self2.lst_files.columns().all_texts(0),
					dest_folder: if dest_folder.is_empty() { None } else { Some(dest_folder) },
					target,
					del_orig: self2.chk_del_orig.is_checked(),
					num_threads: self2.cmb_threads.items().selected_text().unwrap().parse().unwrap(),
				};

				let t0 = util::Timer::start();

				let runner = wnd_run::WndRun::new(&self2.wnd, run_opts);
				runner.show();

				util::prompt::info(self2.wnd.hwnd(), "Process finished",
					&format!("{} file(s) processed in {:.3} secs.",
						self2.lst_files.items().count(), t0.now_ms() / 1000.0));
			}
		});
	}
}
