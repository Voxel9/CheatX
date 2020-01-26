#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xboxkrnl/xboxkrnl.h>
#include <winapi/winbase.h>
#include <xbdm/xbdm.h>

#define __STDC_NO_THREADS__
#include "XbSymbolDatabase/XbSymbolDatabase.h"

#define PHYSICAL_ADDR_BASE	0x00000000
#define PHYSICAL_ADDR_SIZE	0x03FFFFFF

int main() { (void)KeTickCount; return 0; }

static HRESULT __stdcall dump_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\memdump.bin", "wb");
	
	PVOID addr = MmMapIoSpace(PHYSICAL_ADDR_BASE, PHYSICAL_ADDR_SIZE, PAGE_READWRITE);
	
	fwrite(addr, PHYSICAL_ADDR_SIZE, 1, fp);
	
	MmUnmapIoSpace(addr, PHYSICAL_ADDR_SIZE);
	
	fclose(fp);
	
	return XBDM_NOERR;
}

static HRESULT __stdcall poke_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char code_address_buf[16], val_buf[16];
	DWORD code_address, val;
	
	sscanf(szCommand, "pokemem! %s %s", code_address_buf, val_buf);
	
	code_address = strtol(code_address_buf, NULL, 16);
	val = strtol(val_buf, NULL, 16);
	
	PVOID addr = MmMapIoSpace(code_address, 4, PAGE_READWRITE);
	*(DWORD*)addr = val;
	
	MmUnmapIoSpace(addr, 4);
	
	return XBDM_NOERR;
}

struct freeze_entry {
	DWORD physical_addr;
	PVOID mapped_addr;
	DWORD val;
};

struct freeze_entry *freeze_entries = NULL;
int freeze_entry_cnt = 0;

static HRESULT __stdcall freeze_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char code_address_buf[16], val_buf[16];
	DWORD code_address, val;
	
	sscanf(szCommand, "freezemem! %s %s", code_address_buf, val_buf);
	
	code_address = strtol(code_address_buf, NULL, 16);
	val = strtol(val_buf, NULL, 16);
	
	freeze_entry_cnt++;
	freeze_entries = realloc(freeze_entries, sizeof(struct freeze_entry) * freeze_entry_cnt);
	
	freeze_entries[freeze_entry_cnt-1].physical_addr = code_address;
	freeze_entries[freeze_entry_cnt-1].mapped_addr = MmMapIoSpace(code_address, 4, PAGE_READWRITE);
	freeze_entries[freeze_entry_cnt-1].val = val;
	
	return XBDM_NOERR;
}

int search_step = 0;
int entries_cnt = 0;

static HRESULT __stdcall start_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char val_buf[16], condition[16];
	DWORD val;
	sscanf(szCommand, "startsearch! %s %s", condition, val_buf);
	
	val = strtol(val_buf, NULL, 16);
	
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\search0.bin", "wb");
	char outbuf[1024];
	setvbuf(fp, outbuf, _IOFBF, 1024);
	
	search_step = 0;
	entries_cnt = 0;
	
	PVOID addr = MmMapIoSpace(PHYSICAL_ADDR_BASE, PHYSICAL_ADDR_SIZE, PAGE_READWRITE);
	
	if(strcmp(condition, "equals") == 0) {
		for(DWORD i = 0; i < PHYSICAL_ADDR_SIZE; i += 4) {
			if(*(DWORD*)(addr+i) == val) {
				fwrite(&i, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
				entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "not-equals") == 0) {
		for(DWORD i = 0; i < PHYSICAL_ADDR_SIZE; i += 4) {
			if(*(DWORD*)(addr+i) != val) {
				fwrite(&i, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
				entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "less-than") == 0) {
		for(DWORD i = 0; i < PHYSICAL_ADDR_SIZE; i += 4) {
			if(*(DWORD*)(addr+i) < val) {
				fwrite(&i, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
				entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "greater-than") == 0) {
		for(DWORD i = 0; i < PHYSICAL_ADDR_SIZE; i += 4) {
			if(*(DWORD*)(addr+i) > val) {
				fwrite(&i, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
				entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "unknown") == 0) {
		for(DWORD i = 0; i < PHYSICAL_ADDR_SIZE; i += 4) {
			fwrite(&i, sizeof(DWORD), 1, fp);
			fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
			entries_cnt++;
		}
	}
	
	sprintf(szResp, "%d entries found.", entries_cnt);
	
	MmUnmapIoSpace(addr, PHYSICAL_ADDR_SIZE);
	
	fclose(fp);
	
	search_step++;
	
	return XBDM_NOERR;
}

static HRESULT __stdcall continue_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char val_buf[16], condition[16];
	DWORD val;
	sscanf(szCommand, "contsearch! %s %s", condition, val_buf);
	
	val = strtol(val_buf, NULL, 16);
	
	char prevpath[256], fpath[256];
	
	sprintf(prevpath, "\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\search%d.bin", search_step - 1);
	FILE *fprevscan = fopen(prevpath, "rb");
	
	sprintf(fpath, "\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\search%d.bin", search_step);
	FILE *fp = fopen(fpath, "wb");
	char outbuf[1024];
	setvbuf(fp, outbuf, _IOFBF, 1024);
	
	PVOID addr = MmMapIoSpace(PHYSICAL_ADDR_BASE, PHYSICAL_ADDR_SIZE, PAGE_READWRITE);
	
	int new_entries_cnt = 0;
	
	if(strcmp(condition, "equals") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) == val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "not-equals") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) != val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "same") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) == prev_val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "different") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) != prev_val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "less-than") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) < val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "more-than") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) > val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "less") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) < prev_val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	else if(strcmp(condition, "more") == 0) {
		for(int i = 0; i < entries_cnt; i++) {
			DWORD prev_addr, prev_val;
			
			fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
			fread(&prev_val, sizeof(DWORD), 1, fprevscan);
			
			if(*(DWORD*)(addr + prev_addr) > prev_val) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr + prev_addr), sizeof(DWORD), 1, fp);
				new_entries_cnt++;
			}
		}
	}
	
	entries_cnt = new_entries_cnt;
	
	sprintf(szResp, "%d entries found.", new_entries_cnt);
	
	MmUnmapIoSpace(addr, PHYSICAL_ADDR_SIZE);
	
	fclose(fp);
	fclose(fprevscan);
	
	search_step++;
	
	return XBDM_NOERR;
}

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

// CheatX hooked input state
XINPUT_STATE pad_state;

uint32_t getstate_raw = 0;
BYTE oldBytes[6] = {0};
BYTE JMP[6] = {0};

DWORD Hook_XInputGetState(HANDLE hDevice, PXINPUT_STATE pState) {
	DWORD (*Loc_XInputGetState)(HANDLE, PXINPUT_STATE) = (PVOID)getstate_raw;
	
	memcpy((PVOID)getstate_raw, oldBytes, 6);
	DWORD ret = Loc_XInputGetState(hDevice, pState);
	memcpy((PVOID)getstate_raw, JMP, 6);
	return ret;
}

bool first_scan = true;

VOID CDECL scanned_func(const char* library_str, uint32_t library_flag, const char* symbol_str, uint32_t func_addr, uint32_t revision) {
	uint32_t cert_addr = *(DWORD*)0x00010118;
	
	// Debug Stuff
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\scan.txt", "a");
	
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

static VOID InstallGetStateHook() {
	BYTE tempJMP[6] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3};
	
	memcpy(JMP, tempJMP, 6);
	DWORD JMPSize = ((DWORD)Hook_XInputGetState - getstate_raw - 5);
	memcpy(oldBytes, (PVOID)getstate_raw, 6);
	memcpy(&JMP[1], &JMPSize, 4);
	memcpy((PVOID)getstate_raw, JMP, 6);
}

static VOID NTAPI cheat_thread(PKSTART_ROUTINE StartRoutine, PVOID StartContext) {
	// Temp sleep to ensure XBE gets fully loaded before scan
	XSleep(1000);
	
	// Begin Scanning for symbols
	
	// Step 1 & 2
	XbSDBLibraryHeader lib_header;
	XbSDBSectionHeader sec_header;
	
	lib_header.count = XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, NULL);
	sec_header.count = XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, NULL, false);
	
	lib_header.filters = malloc(lib_header.count * sizeof(XbSDBLibrary));
	sec_header.filters = malloc(sec_header.count * sizeof(XbSDBSection));
	
	XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, &lib_header);
	XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, &sec_header, false);
	
	// Step 3
	uint32_t thunk_addr = XbSymbolDatabase_GetKernelThunkAddress((PVOID)0x00010000);
	// Step 4a
	XbSymbolContextHandle xapi_handle;
	
	if(!XbSymbolDatabase_CreateXbSymbolContext(&xapi_handle, scanned_func, lib_header, sec_header, thunk_addr)) {
		XReboot();
	}
	// Step 5
	XbSymbolContext_ScanManual(xapi_handle);
	// Step 6
	XbSymbolContext_ScanLibrary(xapi_handle, lib_header.filters, true);
	// Step 7
	XbSymbolContext_RegisterXRefs(xapi_handle);
	// Step 8
	XbSymbolContext_Release(xapi_handle);
	
	free(sec_header.filters);
	free(lib_header.filters);
	
	InstallGetStateHook();
	
	while(1) {
		XSleep(100);
		
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
