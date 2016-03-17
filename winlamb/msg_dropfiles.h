/**
* Part of WinLamb - Windows Lambda Library
* @author Rodrigo Cesar de Freitas Dias
* @see https://github.com/rodrigocfd/winlamb
*/

#pragma once
#include <algorithm>
#include <string>
#include "wnd_proc.h"
#include "traits_window.h"
#include "traits_dialog.h"

/**
 * msg_dropfiles
 *  wnd_proc
 *   wnd
 */

namespace winlamb {

template<typename traitsT>
class msg_dropfiles : virtual public wnd_proc<traitsT> {
public:
	struct params_dropfiles : public params {
		params_dropfiles(const params& p) { wParam = p.wParam; lParam = p.lParam; }
		HDROP hdrop() const               { return reinterpret_cast<HDROP>(wParam); }

		std::vector<std::basic_string<TCHAR>> get_dropped_files()
		{
			std::vector<std::basic_string<TCHAR>> files(DragQueryFile(hdrop(), 0xFFFFFFFF, nullptr, 0)); // alloc return vector
			for (size_t i = 0; i < files.size(); ++i) {
				files[i].resize(DragQueryFile(hdrop(), static_cast<UINT>(i), nullptr, 0) + 1, L'\0'); // alloc path string
				DragQueryFile(hdrop(), static_cast<UINT>(i), &files[i][0], static_cast<UINT>(files[i].size()));
				files[i].resize(files[i].size() - 1); // trim null
			}
			DragFinish(hdrop());
			std::sort(files.begin(), files.end(), [](const std::basic_string<TCHAR>& a, const std::basic_string<TCHAR>& b)->bool {
				return lstrcmpi(a.c_str(), b.c_str()) < 0;
			});
			return files;
		}
	};
	typedef std::function<typename traitsT::ret_type(params_dropfiles)> func_dropfiles_type;

private:
	func_dropfiles_type _callback;

protected:
	msg_dropfiles()
	{
		on_message(WM_DROPFILES, [this](params p)->typename traitsT::ret_type {
			params_dropfiles pd(p);
			return _callback(pd);
		});
	}

public:
	virtual ~msg_dropfiles() = default;

	void on_dropfiles(func_dropfiles_type callback)
	{
		_callback = std::move(callback);
	}
};

typedef msg_dropfiles<traits_window> window_msg_dropfiles;
typedef msg_dropfiles<traits_dialog> dialog_msg_dropfiles;

}//namespace winlamb