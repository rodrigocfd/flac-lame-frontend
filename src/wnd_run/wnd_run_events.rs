use winsafe::msg;

use super::WndRun;

impl WndRun {
	pub(super) fn events(&self) {
		self.wnd.on().wm_init_dialog({
			let self2 = self.clone();
			move |_: msg::wm::InitDialog| -> bool {
				let nfiles = self2.opts.files.len();
				self2.lbl_status.set_text(&format!("0 of {} files finished...", nfiles)).unwrap();
				self2.pro_status.set_range(0, nfiles as _);

				{
					let mut files_left = self2.files_left.lock().unwrap();
					files_left.reserve(nfiles);
					for idx in 0..nfiles {
						files_left.push(idx); // index of each file
					}
				}

				let max_parallel = std::cmp::min(self2.opts.num_threads, nfiles);
				self2.wnd.hwnd().SetWindowText(
					&format!("Running {} threads", max_parallel)).unwrap();

				for _ in 0..max_parallel {
					std::thread::spawn({
						let self2 = self2.clone();
						move || self2.process_next_file(nfiles)
					});
				}

				true
			}
		});

		// self.wnd.on().wm_close(|| { }); // EndDialog() not called, don't close modal
	}
}
