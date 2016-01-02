/**
 * Part of WOLF - WinAPI Object Lambda Framework
 * @author Rodrigo Cesar de Freitas Dias
 * @see https://github.com/rodrigocfd/wolf
 */

#include <utility>
#include "AccTable.h"
using namespace wolf;

AccTable::~AccTable()
{
	this->destroy();
}

AccTable::AccTable()
	: _hAccel(nullptr)
{
}

AccTable::AccTable(HACCEL hAccel)
	: _hAccel(hAccel)
{
}

AccTable::AccTable(AccTable&& at)
	: _hAccel(at._hAccel)
{
	at._hAccel = nullptr;
}

AccTable& AccTable::operator=(HACCEL hAccel)
{
	this->_hAccel = hAccel;
	return *this;
}

AccTable& AccTable::operator=(AccTable&& at)
{
	std::swap(this->_hAccel, at._hAccel);
	return *this;
}

HACCEL AccTable::hAccel() const
{
	return this->_hAccel;
}

void AccTable::destroy()
{
	if (_hAccel) {
		DestroyAcceleratorTable(_hAccel);
		_hAccel = nullptr;
	}
}

AccTable& AccTable::addChar(WORD commandId, char charKey, Mod modifier)
{
	this->_entries.push_back({
		static_cast<BYTE>(FNOINVERT | static_cast<BYTE>(modifier)),
		static_cast<WORD>(charKey),
		commandId
	});
	return *this;
}

AccTable& AccTable::addKey(WORD commandId, WORD key, Mod modifier)
{
	this->_entries.push_back({
		static_cast<BYTE>(FNOINVERT | FVIRTKEY | static_cast<BYTE>(modifier)),
		key,
		commandId
	});
	return *this;
}

bool AccTable::create(DWORD *lastError)
{
	if (this->_entries.empty()) return true; // nothing to do

	this->_hAccel = CreateAcceleratorTable(this->_entries.data(),
		static_cast<int>(this->_entries.size()));
	if (lastError) *lastError = this->_hAccel ? 0 : GetLastError();
	if (this->_hAccel) this->_entries.clear();
	return this->_hAccel != nullptr;
}

int AccTable::translate(HWND hWnd, MSG& msg)
{
	return TranslateAccelerator(hWnd, this->_hAccel, &msg);
}