#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod ids;
mod wnd_main;

fn main() {
	if let Err(e) = wnd_main::WndMain::new().run() {
		eprintln!("{}", e);
	}
}
