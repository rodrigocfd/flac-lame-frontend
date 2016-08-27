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
 *                     +-- msg_dropfiles<traits_window> <-- window_msg_dropfiles
 * wnd <-- wnd_proc <--+
 *                     +-- msg_dropfiles<traits_dialog> <-- dialog_msg_dropfiles
 */

namespace winlamb {

template<typename traitsT>
class msg_dropfiles : virtual public wnd_proc<traitsT> {
public:
	struct params_dropfiles : public params {
		params_dropfiles(const params& p) : params(p) { }
		HDROP hdrop() const               { return reinterpret_cast<HDROP>(this->params::wParam); }

		std::vector<std::basic_string<TCHAR>> get_dropped_files() const
		{
			std::vector<std::basic_string<TCHAR>> files(DragQueryFile(this->hdrop(), 0xFFFFFFFF, nullptr, 0)); // alloc return vector
			for (size_t i = 0; i < files.size(); ++i) {
				files[i].resize(DragQueryFile(this->hdrop(), static_cast<UINT>(i), nullptr, 0) + 1, TEXT('\0')); // alloc path string
				DragQueryFile(this->hdrop(), static_cast<UINT>(i), &files[i][0], static_cast<UINT>(files[i].size()));
				files[i].resize(files[i].size() - 1); // trim null
			}
			DragFinish(this->hdrop());
			std::sort(files.begin(), files.end(),
				[](const std::basic_string<TCHAR>& a, const std::basic_string<TCHAR>& b)->bool {
					return lstrcmpi(a.c_str(), b.c_str()) < 0;
				});
			return files;
		}

		POINT get_drop_point() const
		{
			POINT pt = { 0, 0 };
			DragQueryPoint(this->hdrop(), &pt);
			return pt;
		}
	};
	typedef std::function<typename traitsT::ret_type(params_dropfiles)> func_dropfiles_type;

private:
	func_dropfiles_type _callback;

protected:
	msg_dropfiles()
	{
		this->wnd_proc::on_message(WM_DROPFILES, [this](params p)->typename traitsT::ret_type {
			params_dropfiles pd(p);
			return this->_callback(pd);
		});
	}

public:
	virtual ~msg_dropfiles() = default;

	void on_dropfiles(func_dropfiles_type callback)
	{
		this->_callback = std::move(callback);
	}
};

typedef msg_dropfiles<traits_window> window_msg_dropfiles;
typedef msg_dropfiles<traits_dialog> dialog_msg_dropfiles;

}//namespace winlamb