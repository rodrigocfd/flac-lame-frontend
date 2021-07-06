use std::error::Error;
use winsafe::{self as w, co};

use crate::mapped_file::{MappedFile, MappedFileAccess};

pub struct Ini {
	pub sections: Vec<IniSection>,
}

pub struct IniSection {
	pub name: String,
	pub entries: Vec<IniEntry>,
}

pub struct IniEntry {
	pub key: String,
	pub val: String,
}

impl Ini {
	pub fn parse_str(contents: &str) -> Ini {
		let mut sections = Vec::default();
		let mut cur_section = IniSection {
			name: "".to_owned(),
			entries: Vec::default(),
		};

		for line in contents.lines() {
			let line = line.trim();

			if line.starts_with('[') && line.ends_with(']') {
				if cur_section.name != "" || !cur_section.entries.is_empty() {
					sections.push(cur_section);
				}
				cur_section = IniSection {
					name: line[1..line.len() - 1].to_owned(),
					entries: Vec::default(),
				};
				continue;
			}

			if let Some(eq_idx) = line.find('=') {
				cur_section.entries.push(IniEntry {
					key: line[..eq_idx].to_owned(),
					val: line[eq_idx + 1..].to_owned(),
				});
			}
		}
		if cur_section.name != "" || !cur_section.entries.is_empty() {
			sections.push(cur_section);
		}

		Self { sections }
	}

	pub fn parse_bytes(bytes: &[u8]) -> Result<Ini, Box<dyn Error>> {
		Ok(
			Self::parse_str(&w::WString::parse_str(bytes)?.to_string()),
		)
	}

	pub fn parse_from_file(ini_path: &str) -> Result<Ini, Box<dyn Error>> {
		let fin = MappedFile::open(ini_path, MappedFileAccess::Read)?;
		Self::parse_bytes(fin.as_slice())
	}

	pub fn serialize_to_str(&self) -> String {
		let mut tot_size = 0;
		for (idx, section) in self.sections.iter().enumerate() {
			tot_size += section.name.len() + 2 + 2;
			for entry in section.entries.iter() {
				tot_size += entry.key.len() + 1 + entry.val.len() + 2;
			}
			if idx < self.sections.len() - 1 {
				tot_size += 2;
			}
		}

		let mut buf = String::with_capacity(tot_size);
		for (idx, section) in self.sections.iter().enumerate() {
			buf.push('[');
			buf.push_str(&section.name);
			buf.push_str("]\r\n");

			for entry in section.entries.iter() {
				buf.push_str(&entry.key);
				buf.push('=');
				buf.push_str(&entry.val);
				buf.push_str("\r\n");
			}

			if idx < self.sections.len() - 1 {
				buf.push_str("\r\n");
			}
		}
		buf
	}

	pub fn serialize_to_bytes(&self) -> Vec<u8> {
		self.serialize_to_str().into_bytes()
	}

	pub fn serialize_to_file(&self, ini_path: &str) -> Result<(), Box<dyn Error>> {
		let (fout, _) = w::HFILE::CreateFile(ini_path, co::GENERIC::WRITE,
			co::FILE_SHARE::NONE, None, co::DISPOSITION::CREATE_ALWAYS,
			co::FILE_ATTRIBUTE::NORMAL, None)?;
		defer! { fout.CloseHandle().unwrap(); }

		fout.WriteFile(&self.serialize_to_bytes(), None)?;
		Ok(())
	}
}
