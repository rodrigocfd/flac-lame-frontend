use winsafe::gui;

mod wnd_run_events;
mod wnd_run_funcs;

#[derive(Clone)]
pub struct WndRun {
	wnd:        gui::WindowModal,
	lbl_status: gui::Label,
	pro_status: gui::ProgressBar,
}
