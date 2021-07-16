#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

#[macro_use]
mod defer;

mod ids;
mod util;
mod wnd_main;
mod wnd_run;

fn main() {
	// if let Err(e) = wnd_main::WndMain::new().run() {
	// 	eprintln!("{}", e);
	// }


	use winsafe as o;
	o::HINSTANCE::NULL.EnumResourceTypes(|t| {
		match &t {
			o::RtStr::Rt(id) => println!("TYPE ID {}", *id),
			o::RtStr::Str(s) => println!("TYPE STR {}", s),
		}
		o::HINSTANCE::NULL.EnumResourceNames(t.clone(), |n| {
			match &n {
				o::IdStr::Id(id) => println!("    NAME ID {}", *id),
				o::IdStr::Str(s) => println!("    NAME STR {}", s),
			}
			o::HINSTANCE::NULL.EnumResourceLanguages(t.clone(), n, |l| {
				// println!("        LANG {} {}", l.primary_lang_id(), l.sub_lang_id());
				println!("        {}", u16::from(l));
				true
			}).unwrap();
			true
		}).unwrap();
		true
	}).unwrap();
}
