//
// Automation to aid WM_INITMENUPOPUP handling.
// Sunday afternoon, February 24, 2013.
//

#include <Windows.h>

class InitMenuPopup {
public:
	explicit InitMenuPopup(WPARAM wp) : _hMenu((HMENU)wp) { }

	bool           isFirst(int cmdItemId)               { return ::GetMenuItemID(_hMenu, 0) == cmdItemId; }
	InitMenuPopup& enable(int cmdItemId, bool doEnable) { ::EnableMenuItem(_hMenu, cmdItemId, MF_BYCOMMAND | ((doEnable) ? MF_ENABLED : MF_GRAYED)); return *this; }
	InitMenuPopup& setDefault(int cmdItemId)            { ::SetMenuDefaultItem(_hMenu, cmdItemId, MF_BYCOMMAND); return *this; }

private:
	HMENU _hMenu;
};