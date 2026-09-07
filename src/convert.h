#pragma once
#include <optional>
#include "../windlg/lib.hpp"

namespace convert {

	void to_wav(
		wd::StrView lamePath,
		wd::StrView flacPath,
		wd::StrView srcFile,
		std::optional<wd::StrView> destFolder,
		bool delSrc);

	void to_flac(
		wd::StrView lamePath,
		wd::StrView flacPath,
		wd::StrView srcFile,
		std::optional<wd::StrView> destFolder,
		bool delSrc,
		wd::StrView quality);

	void to_mp3(
		wd::StrView lamePath,
		wd::StrView flacPath,
		wd::StrView srcFile,
		std::optional<wd::StrView> destFolder,
		bool delSrc,
		wd::StrView quality,
		bool isVbr);

}
