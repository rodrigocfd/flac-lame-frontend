use winsafe::gui;

mod wnd_main_events;
mod wnd_main_funcs;

#[derive(Clone)]
pub struct WndMain {
	wnd: gui::WindowMain,
}
