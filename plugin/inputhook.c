#include "inputhook.h"

#include "XbSymbolDatabase/XbSymbolDatabase.h"

// CheatX hooked xinput state
XINPUT_GAMEPAD pad;

uint32_t Addr_XInputGetState = 0;
BYTE old_bytes[6];
BYTE jump_hook[6] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3};

DWORD WINAPI Hook_XInputGetState(HANDLE hDevice, PXINPUT_STATE pState) {
    // Restore original bytes to beginning of function
	memcpy((PVOID)Addr_XInputGetState, old_bytes, 6);
    
    // Execute the original function
    DWORD WINAPI (*Call_XInputGetState)(HANDLE, PXINPUT_STATE) = (PVOID)Addr_XInputGetState;
	DWORD ret = Call_XInputGetState(hDevice, pState);
    
    // Restore jump hook
	memcpy((PVOID)Addr_XInputGetState, jump_hook, 6);
	
    // Obtain gamepad state for reading in CheatX
	memcpy(&pad, &pState->Gamepad, sizeof(XINPUT_GAMEPAD));
	
	return ret;
}

VOID CDECL scanned_func(const char* library_str, uint32_t library_flag, const char* symbol_str, uint32_t func_addr, uint32_t revision) {
	uint32_t cert_addr = *(DWORD*)0x00010118;
	
	// Debug Stuff
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\scan.txt", "a");
	
	static bool first_scan = true;
	if(first_scan) {
		fprintf(fp, "\nTitle ID: 0x%08x\n", *(DWORD*)(cert_addr + 0x8));
		first_scan = false;
	}
	
	fprintf(fp, "%s - 0x%08x\n", symbol_str, func_addr);
	
	fclose(fp);
	// End Debug Stuff
	
	if(strcmp("XInputGetState", symbol_str) == 0) {
		Addr_XInputGetState = func_addr;
	}
}

VOID InstallGetStateHook() {
    // Scan for XInputGetState location first
    XbSymbolScan((PVOID)0x00010000, scanned_func, false);
    
    // Store first 6 original bytes where jump will be written
	memcpy(old_bytes, (PVOID)Addr_XInputGetState, 6);
    
    // Calculate relative jump size and overwrite NOP bytes with the value
    DWORD jump_size = ((DWORD)Hook_XInputGetState - Addr_XInputGetState - 5);
	memcpy(&jump_hook[1], &jump_size, 4);
    
    // Finally, write jump instruction to beginning of XInputGetState
	memcpy((PVOID)Addr_XInputGetState, jump_hook, 6);
}
