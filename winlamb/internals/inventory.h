/**
 * Part of WinLamb - Win32 API Lambda Library
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/winlamb
 */

#pragma once
#include <functional>
#include <vector>
#include "../params.h"

namespace wl {
namespace internals {

class inventory final {
public:
	using funcT = std::function<LONG_PTR(params&)>; // works for both LRESULT and LONG_PTR

private:
	template<typename idT>
	class _depot final {
	private:
		std::vector<std::pair<idT, funcT>> _msgUnits;
	public:
		_depot() {
			this->_msgUnits.reserve(21); // arbitrary, to save realloc time
			this->_msgUnits.emplace_back(); // 1st element is sentinel room
		}

		funcT* add(idT id, funcT func) {
			this->_msgUnits.emplace_back(id, std::move(func)); // reverse search: messages can be overwritten
			return &this->_msgUnits.back().second;
		}

		funcT* find(idT id) {
			this->_msgUnits[0].first = id; // sentinel for reverse linear search
			std::pair<idT, funcT>* revRunner = &this->_msgUnits.back();
			while (revRunner->first != id) --revRunner;
			return revRunner == &this->_msgUnits[0] ?
				nullptr : &revRunner->second;
		}
	};

	_depot<UINT> _messages;
	_depot<WORD> _commands;
	_depot<std::pair<UINT_PTR, UINT>> _notifies;

public:
	funcT* find_func(const params& p) {
		funcT* func = nullptr;
		if (p.message == WM_COMMAND) { // if user adds raw WM_COMMAND or WM_NOTIFY, they will never be called
			return this->_commands.find(LOWORD(p.wParam));
		} else if (p.message == WM_NOTIFY) {
			return this->_notifies.find({
				reinterpret_cast<NMHDR*>(p.lParam)->idFrom,
				reinterpret_cast<NMHDR*>(p.lParam)->code
			});
		}
		return this->_messages.find(p.message);
	}

	void add_message(UINT msg, funcT func) {
		this->_messages.add(msg, std::move(func));
	}

	void add_message(std::initializer_list<UINT> msgs, funcT func) {
		const UINT* pMsgs = msgs.begin();
		funcT* pFirstFunc = this->_messages.add(pMsgs[0], std::move(func)); // store user func once
		for (size_t i = 1; i < msgs.size(); ++i) {
			if (pMsgs[i] != pMsgs[0]) { // avoid overwriting
				this->add_message(pMsgs[i], [pFirstFunc](params p)->LONG_PTR {
					return (*pFirstFunc)(p); // store light wrapper to 1st func
				});
			}
		}
	}

	void add_command(WORD cmd, funcT func) {
		this->_commands.add(cmd, std::move(func));
	}

	void add_command(std::initializer_list<WORD> cmds, funcT func) {
		const WORD* pCmds = cmds.begin();
		funcT* pFirstFunc = this->_commands.add(pCmds[0], std::move(func));
		for (size_t i = 1; i < cmds.size(); ++i) {
			if (pCmds[i] != pCmds[0]) {
				this->add_command(pCmds[i], [pFirstFunc](params p)->LONG_PTR {
					return (*pFirstFunc)(p);
				});
			}
		}
	}

	void add_notify(UINT_PTR idFrom, UINT code, funcT func) {
		this->_notifies.add({idFrom, code}, std::move(func));
	}

	void add_notify(UINT_PTR idFrom, std::initializer_list<UINT> codes, funcT func) {
		const UINT* pCodes = codes.begin();
		funcT* pFirstFunc = this->_notifies.add({idFrom, pCodes[0]}, std::move(func));
		for (size_t i = 1; i < codes.size(); ++i) {
			if (pCodes[i] != pCodes[0]) {
				this->add_notify(idFrom, pCodes[i], [pFirstFunc](params p)->LONG_PTR {
					return (*pFirstFunc)(p);
				});
			}
		}
	}

	void add_notify(std::pair<UINT_PTR, UINT> id, funcT func) {
		this->_notifies.add({id.first, id.second}, std::move(func));
	}

	void add_notify(std::initializer_list<std::pair<UINT_PTR, UINT>> ids, funcT func) {
		const std::pair<UINT_PTR, UINT>* pIds = ids.begin();
		funcT* pFirstFunc = this->_notifies.add(pIds[0], std::move(func));
		for (size_t i = 1; i < ids.size(); ++i) {
			if (pIds[i] != pIds[0]) {
				this->add_notify(pIds[i], [pFirstFunc](params p)->LONG_PTR {
					return (*pFirstFunc)(p);
				});
			}
		}
	}
};

}//namespace internals
}//namespace wl