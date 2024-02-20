use winsafe::{self as w};

/// UiSettings section data.
pub struct UiSettings {
	pub target: u8,
	pub mp3enc: u8,
	pub cbr: u8,
	pub vbr: u8,
	pub flaclvl: u8,
	pub delorig: bool,
}

/// Returns the paths of LAME and FLAC tools, read from the local INI file.
#[must_use]
pub fn read_tool_paths() -> w::AnyResult<(String, String)> {
	let ini_file_path = internal::ini_path()?;
	let lame_path = match w::GetPrivateProfileString("Tools", "lame", &ini_file_path)? {
		Some(lame_path) => lame_path,
		None => {
			return Err(format!("LAME path not found at:\n{}", ini_file_path).into());
		},
	};
	let flac_path = match w::GetPrivateProfileString("Tools", "flac", &ini_file_path)? {
		Some(lame_path) => lame_path,
		None => {
			return Err(format!("FLAC path not found at:\n{}", ini_file_path).into());
		},
	};

	if !w::path::exists(&lame_path) {
		Err(format!("LAME tool not found at:\n{}", lame_path).into())
	} else if !w::path::exists(&flac_path) {
		Err(format!("FLAC tool not found at:\n{}", flac_path).into())
	} else {
		Ok((lame_path, flac_path))
	}
}

/// Reads the `UiSettings` section from the INI file.
#[must_use]
pub fn read_ui_settings() -> w::AnyResult<UiSettings> {
	let ini_path = internal::ini_path()?;
	Ok(UiSettings {
		target: internal::read_num(&ini_path, "UiSettings", "target", 0)?,
		mp3enc: internal::read_num(&ini_path, "UiSettings", "mp3enc", 1)?,
		cbr: internal::read_num(&ini_path, "UiSettings", "cbr", 8)?,
		vbr: internal::read_num(&ini_path, "UiSettings", "vbr", 4)?,
		flaclvl: internal::read_num(&ini_path, "UiSettings", "flaclvl", 7)?,
		delorig: internal::read_bool(&ini_path, "UiSettings", "delorig", false)?,
	})
}

/// Writes the `UiSettings` to the INI file.
pub fn serialize_ui_settings(ui_settings: &UiSettings) -> w::AnyResult<()> {
	let ini_path = internal::ini_path()?;
	internal::serialize_num(&ini_path, "UiSettings", "target", ui_settings.target)?;
	internal::serialize_num(&ini_path, "UiSettings", "mp3enc", ui_settings.mp3enc)?;
	internal::serialize_num(&ini_path, "UiSettings", "cbr", ui_settings.cbr)?;
	internal::serialize_num(&ini_path, "UiSettings", "vbr", ui_settings.vbr)?;
	internal::serialize_num(&ini_path, "UiSettings", "flaclvl", ui_settings.flaclvl)?;
	internal::serialize_bool(&ini_path, "UiSettings", "delorig", ui_settings.delorig)?;
	Ok(())
}

mod internal {
	use winsafe::{self as w};

	/// Returns the path of the INI configuration file.
	#[must_use]
	pub fn ini_path() -> w::AnyResult<String> {
		Ok( format!("{}\\flac-lame-frontend.ini", w::path::exe_path()?) )
	}

	/// Parses a number from the INI file.
	#[must_use]
	pub fn read_num(ini_path: &str, section: &str, key: &str, def_val: u8) -> w::AnyResult<u8> {
		let def_val_str = def_val.to_string();
		let str_val = w::GetPrivateProfileString(section, key, ini_path)?
			.unwrap_or(def_val_str);
		str_val.parse::<u8>()
			.map_err(|e| e.into())
	}

	/// Parses a boolean from the INI file.
	#[must_use]
	pub fn read_bool(ini_path: &str, section: &str, key: &str, def_val: bool) -> w::AnyResult<bool> {
		let def_val_str = if def_val { "1" } else { "0" }.to_owned();
		let str_val = w::GetPrivateProfileString(section, key, ini_path)?
			.unwrap_or(def_val_str);
		Ok(match str_val.as_str() {
			"0" => false,
			_ => true,
		})
	}

	/// Serializes a number to the INI file.
	pub fn serialize_num(ini_path: &str, section: &str, key: &str, val: u8) -> w::AnyResult<()> {
		w::WritePrivateProfileString(section, Some(key), Some(&val.to_string()), ini_path)
			.map_err(|e| e.into())
	}

	/// Serializes a boolean to the INI file.
	pub fn serialize_bool(ini_path: &str, section: &str, key: &str, val: bool) -> w::AnyResult<()> {
		w::WritePrivateProfileString(section, Some(key), Some(if val { "1" } else { "0" }), ini_path)
			.map_err(|e| e.into())
	}
}
