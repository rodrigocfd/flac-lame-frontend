use std::sync::{Arc, Mutex};
use winsafe::{gui, shell};

mod wnd_run_events;
mod wnd_run_funcs;

#[derive(Clone)]
pub struct WndRun {
	wnd:           gui::WindowModal,
	lbl_status:    gui::Label,
	pro_status:    gui::ProgressBar,
	itbl:          shell::ITaskbarList4,
	opts:          Opts,
	files_process: Arc<Mutex<FilesProcess>>,
}

#[derive(PartialEq, Eq, Clone)]
pub enum Target {
	Mp3(Mp3Enc, String), // encoding, quality
	Flac(String), // quality
	Wav,
}

#[derive(PartialEq, Eq, Copy, Clone)]
pub enum Mp3Enc { Cbr, Vbr }

#[derive(Clone)]
pub struct Opts {
	pub files:       Vec<String>,
	pub dest_folder: Option<String>,
	pub target:      Target,
	pub del_orig:    bool,
	pub num_threads: usize,
}

struct FilesProcess {
	pub idx_files_left: Vec<usize>,
	pub num_files_done: usize,
}
