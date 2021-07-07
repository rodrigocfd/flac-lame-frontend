use winsafe::{self as w, co, gui, msg};

use crate::ids;
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

				[".mp3", ".flac", ".wav"].iter().for_each(|ext: &&str| {
					dropped_files.iter().for_each(|file: &String| {
						let mut file = file.clone();

						if w::GetFileAttributes(&file).unwrap().has(co::FILE_ATTRIBUTE::DIRECTORY) {
							if !file.ends_with('\\') {
								file.push('\\');
							}
							file.push('*');
							file.push_str(*ext);

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

				self2.add_files(&all_files).unwrap();
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
	}
}
