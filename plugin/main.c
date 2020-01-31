#include "main.h"

#include "memfuncs.h"
#include "searchfuncs.h"
#include "inputhook.h"

int main() { (void)KeTickCount; return 0; }

static VOID NTAPI cheat_thread(PKSTART_ROUTINE StartRoutine, PVOID StartContext) {
	// Temp sleep to ensure XBE gets fully loaded before scan
	XSleep(1000);
	
	LocateGetStateFunc();
	InstallGetStateHook();
	
	while(1) {
		XSleep(100);
		
		// dpad up
		if(pad.wButtons & 0x00000001) {
			
		}
		
		for(int i = 0; i < freeze_entry_cnt; i++) {
			*(DWORD*)freeze_entries[i].mapped_addr = freeze_entries[i].val;
		}
	}
}

void DxtEntry(ULONG *pfUnload) {
	DmRegisterCommandProcessor("DUMPMEM", dump_memory);
	DmRegisterCommandProcessor("POKEMEM", poke_memory);
	DmRegisterCommandProcessor("FREEZEMEM", freeze_memory);
	DmRegisterCommandProcessor("STARTSEARCH", start_search);
	DmRegisterCommandProcessor("CONTSEARCH", continue_search);
	
	HANDLE handle, id;
	NTSTATUS status = PsCreateSystemThreadEx(&handle, 0, 8192, 0, &id, (PKSTART_ROUTINE)NULL, (PVOID)NULL, FALSE, FALSE, cheat_thread);
	
	*pfUnload = FALSE;
}
