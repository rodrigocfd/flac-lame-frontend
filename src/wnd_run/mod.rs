use std::sync::{Arc, Mutex};
use winsafe::{self as w, gui};

mod wnd_run_ctor;
mod wnd_run_wm;
mod wnd_run_process;

#[derive(Clone)]
pub struct WndRun {
	wnd:           gui::WindowModal,
	lbl_status:    gui::Label,
	pro_status:    gui::ProgressBar,
	itbl:          w::ITaskbarList4,
	opts:          Opts,
	files_process: Arc<Mutex<FilesProcess>>,
}

unsafe impl Send for WndRun {}

#[derive(PartialEq, Eq, Clone)]
pub enum Target {
	Mp3(Mp3Enc, String), // encoding, quality
	Flac(String), // quality
	Wav,
}

#[derive(PartialEq, Eq, Clone, Copy)]
pub enum Mp3Enc { Cbr, Vbr }

#[derive(Clone)]
pub struct Opts {
	pub lame_path:   String,
	pub flac_path:   String,
	pub files:       Vec<String>,
	pub dest_folder: Option<String>,
	pub target:      Target,
	pub del_orig:    bool,
	pub num_threads: usize,
}

#[derive(Default)]
struct FilesProcess {
	pub idx_files_left: Vec<usize>,
	pub num_files_done: usize,
}
