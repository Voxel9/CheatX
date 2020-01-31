#include "inputhook.h"

// CheatX hooked input state
XINPUT_GAMEPAD pad;

uint32_t getstate_raw = 0;
BYTE oldBytes[6] = {0};
BYTE JMP[6] = {0};

DWORD WINAPI Hook_XInputGetState(HANDLE hDevice, PXINPUT_STATE pState) {
	DWORD WINAPI (*Loc_XInputGetState)(HANDLE, PXINPUT_STATE) = (PVOID)getstate_raw;
	
	memcpy((PVOID)getstate_raw, oldBytes, 6);
	DWORD ret = Loc_XInputGetState(hDevice, pState);
	memcpy((PVOID)getstate_raw, JMP, 6);
	
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
		getstate_raw = func_addr;
	}
}

VOID LocateGetStateFunc() {
	XbSDBLibraryHeader lib_header;
	XbSDBSectionHeader sec_header;
	
	lib_header.count = XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, NULL);
	sec_header.count = XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, NULL, false);
	
	lib_header.filters = malloc(lib_header.count * sizeof(XbSDBLibrary));
	sec_header.filters = malloc(sec_header.count * sizeof(XbSDBSection));
	
	XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, &lib_header);
	XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, &sec_header, false);
	
	uint32_t thunk_addr = XbSymbolDatabase_GetKernelThunkAddress((PVOID)0x00010000);
	XbSymbolContextHandle xapi_handle;
	
	if(!XbSymbolDatabase_CreateXbSymbolContext(&xapi_handle, scanned_func, lib_header, sec_header, thunk_addr)) {
		XReboot();
	}
	
	XbSymbolContext_ScanManual(xapi_handle);
	
	XbSymbolContext_ScanLibrary(xapi_handle, lib_header.filters, true);
	
	XbSymbolContext_RegisterXRefs(xapi_handle);
	
	XbSymbolContext_Release(xapi_handle);
	
	free(sec_header.filters);
	free(lib_header.filters);
}

VOID InstallGetStateHook() {
	BYTE tempJMP[6] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3};
	
	memcpy(JMP, tempJMP, 6);
	DWORD JMPSize = ((DWORD)Hook_XInputGetState - getstate_raw - 5);
	memcpy(oldBytes, (PVOID)getstate_raw, 6);
	memcpy(&JMP[1], &JMPSize, 4);
	memcpy((PVOID)getstate_raw, JMP, 6);
}
