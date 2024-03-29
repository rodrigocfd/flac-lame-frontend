#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

mod ids;
mod util;
mod wnd_main;
mod wnd_run;

fn main() {
	if let Err(e) = (|| wnd_main::WndMain::new()?.run())() {
		eprintln!("{}", e);
	}
}
