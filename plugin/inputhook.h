#ifndef __INPUTHOOK_H__
#define __INPUTHOOK_H__

#include "main.h"

// Xbox XInput structs, taken from Cxbx-Reloaded
// (https://github.com/Cxbx-Reloaded/Cxbx-Reloaded/blob/develop/src/core/hle/XAPI/Xapi.h)

typedef struct _XINPUT_GAMEPAD {
	WORD    wButtons;
	BYTE    bAnalogButtons[8];
	SHORT   sThumbLX;
	SHORT   sThumbLY;
	SHORT   sThumbRX;
	SHORT   sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_STATE {
	DWORD dwPacketNumber;
	
	union {
		XINPUT_GAMEPAD Gamepad;
	};
} XINPUT_STATE, *PXINPUT_STATE;

extern XINPUT_GAMEPAD pad;

VOID InstallGetStateHook();

#endif
