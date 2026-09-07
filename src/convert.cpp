#include "convert.h"

static void exec_and_delete(wd::StrView cmdLine, wd::StrView srcFile, bool delSrc) {
#ifdef _DEBUG
	// Debug summary of operations about to be performed.
	wd::dbg(L"RUNNING:\n%s", cmdLine);
	if (delSrc)
		wd::dbg(L"...AND DELETING:\n%s", srcFile);
#endif

	wd::run_cmd_sync(cmdLine);
	if (delSrc) {
		if (!DeleteFileW(srcFile.c_str())) [[unlikely]] {
			throw wd::WinErr{GetLastError(), wd::str::fmt(L"Failed to delete file:\n%s", srcFile)};
		}
	}
}

struct TypeCheck final {
	explicit TypeCheck(wd::StrView file) {
		std::wstring fileUp{file};
		wd::str::to_upper(fileUp);
		isWav  = wd::str::ends_with(fileUp, L".WAV");
		isFlac = wd::str::ends_with(fileUp, L".FLAC");
		isMp3  = wd::str::ends_with(fileUp, L".MP3");
	}
	bool isWav=false, isFlac=false, isMp3=false;
};

void convert::to_wav(
	wd::StrView lamePath,
	wd::StrView flacPath,
	wd::StrView srcFile,
	std::optional<wd::StrView> destFolder,
	bool delSrc)
{
	std::optional<std::wstring> destFolder2{};
	if (destFolder.has_value() && wd::str::neq_i(wd::file::dir_from(srcFile), destFolder.value())) // different dir from src?
		destFolder2.emplace(destFolder.value());

	const TypeCheck srcFileType{srcFile};
	std::wstring cmdLine{};

	if (srcFileType.isMp3) {
		cmdLine = wd::str::fmt(L"\"%s\" --decode \"%s\"", lamePath, srcFile);
	} else if (srcFileType.isFlac) {
		cmdLine = wd::str::fmt(L"\"%s\" -d \"%s\"", flacPath, srcFile);
		if (destFolder2.has_value()) {
			cmdLine.append(L" -o"); // different destination folder requires flag
		}
	} else {
		throw wd::WinErr{ERROR_INVALID_PARAMETER, wd::str::fmt(L"Nonsense conversion from WAV to WAV:\n%s", srcFile)};
	}

	if (destFolder2.has_value()) { // different destination folder
		std::wstring destWavPath = wd::file::remove_ext(srcFile) + L".wav";
		cmdLine.append( wd::str::fmt(L" \"%s\\%s\"", destFolder2.value(), wd::file::filename_from(destWavPath)) );
	}

	exec_and_delete(cmdLine, srcFile, delSrc);
}

void convert::to_flac(
	wd::StrView lamePath,
	wd::StrView flacPath,
	wd::StrView srcFile,
	std::optional<wd::StrView> destFolder,
	bool delSrc,
	wd::StrView quality)
{
	std::optional<std::wstring> destFolder2{};
	if (destFolder.has_value() && wd::str::neq_i(wd::file::dir_from(srcFile), destFolder.value())) // different dir from src?
		destFolder2.emplace(destFolder.value());

	const TypeCheck srcFileType{srcFile};
	std::wstring srcFile2{srcFile};

	if (srcFileType.isMp3 || srcFileType.isFlac) { // needs intermediary WAV conversion
		if (srcFileType.isMp3) { // MP3 to FLAC
			to_wav(lamePath, flacPath, srcFile2, destFolder2, delSrc); // send WAV straight to new folder, if any
		} else if (srcFileType.isFlac) { // FLAC to FLAC
			to_wav(lamePath, flacPath, srcFile2, destFolder2, // send WAV straight to new folder, if any
				destFolder2.has_value() ? delSrc : true); // if same destination folder, then delete FLAC (will be replaced)
		}

		if (destFolder2.has_value()) { // different destination folder
			srcFile2 = wd::str::fmt(L"%s\\%s", destFolder2.value(), wd::file::filename_from(srcFile2));
			destFolder2 = std::nullopt;
		}
		srcFile2 = wd::file::remove_ext(srcFile2) + L".wav"; // our source is now a WAV
		delSrc = true; // delete intermediary WAV
	} else if (!srcFileType.isFlac && !srcFileType.isWav) {
		throw wd::WinErr{ERROR_INVALID_PARAMETER, wd::str::fmt(L"Not a FLAC/WAV:\n%s", srcFile)};
	}

	std::wstring cmdLine = wd::str::fmt(L"\"%s\" -%s -V --no-seektable \"%s\"", flacPath, quality, srcFile2);

	if (destFolder2.has_value()) { // different destination folder
		std::wstring destFlacPath{srcFile2};
		srcFile2 = wd::file::remove_ext(srcFile2) + L".flac";
		cmdLine.append( wd::str::fmt(L" -o \"%s\\%s\"", destFolder2.value(), wd::file::filename_from(destFlacPath)) );
	}

	exec_and_delete(cmdLine, srcFile2, delSrc);
}

void convert::to_mp3(
	wd::StrView lamePath,
	wd::StrView flacPath,
	wd::StrView srcFile,
	std::optional<wd::StrView> destFolder,
	bool delSrc,
	wd::StrView quality,
	bool isVbr)
{
	std::optional<std::wstring> destFolder2{};
	if (destFolder.has_value() && wd::StrView{wd::file::dir_from(srcFile)}.neq_i(destFolder.value())) // different dir from src?
		destFolder2.emplace(destFolder.value());

	const TypeCheck srcFileType{srcFile};
	std::wstring srcFile2{srcFile};

	if (srcFileType.isFlac || srcFileType.isMp3) { // needs intermediary WAV conversion
		if (srcFileType.isFlac) { // FLAC to MP3
			to_wav(lamePath, flacPath, srcFile2, destFolder2, delSrc); // send WAV straight to new folder, if any
		} else if (srcFileType.isMp3) { // MP3 to MP3
			to_wav(lamePath, flacPath, srcFile2, destFolder2, // send WAV straight to new folder, if any
				destFolder2.has_value() ? delSrc : true); // if same destination folder, then delete MP3 (will be replaced)
		}

		if (destFolder2.has_value()) { // different destination folder
			srcFile2 = wd::str::fmt(L"%s\\%s", destFolder2.value(), wd::file::filename_from(srcFile2));
			destFolder2 = std::nullopt;
		}
		srcFile2 = wd::file::remove_ext(srcFile2) + L".wav"; // our source is now a WAV
		delSrc = true; // delete intermediary WAV
	} else if (!srcFileType.isFlac && !srcFileType.isMp3 && !srcFileType.isWav) {
		throw wd::WinErr{ERROR_INVALID_PARAMETER, wd::str::fmt(L"Not a FLAC/MP3/WAV\n%s", srcFile)};
	}

	std::wstring cmdLine = wd::str::fmt(L"\"%s\" -%s%s --noreplaygain \"%s\"",
		lamePath, (isVbr ? L"V" : L"b"), quality, srcFile2);

	if (destFolder2.has_value()) { // different destination folder
		std::wstring destMp3Path{srcFile2};
		srcFile2 = wd::file::remove_ext(srcFile2) + L".mp3";
		cmdLine.append( wd::str::fmt(L" \"%s\\%s\"", destFolder2.value(), wd::file::filename_from(destMp3Path)) );
	}

	exec_and_delete(cmdLine, srcFile2, delSrc);
}
