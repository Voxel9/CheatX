#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xboxkrnl/xboxkrnl.h>
#include <winapi/winbase.h>
#include <xbdm/xbdm.h>

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

unsigned int xrefs_cnt = 0;
unsigned int has_this_func_even_been_called = 0;

// Simple function to check xrefs are being scanned properly
static HRESULT __stdcall symbolz(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	sprintf(szResp, "Num of xrefs: %u\nNum of scan func calls: %u", xrefs_cnt, has_this_func_even_been_called);
	
	return XBDM_NOERR;
}

VOID CDECL scanned_func(const char* library_str, uint32_t library_flag, const char* symbol_str, uint32_t func_addr, uint32_t revision) {
	has_this_func_even_been_called++;
}

static VOID NTAPI cheat_thread(PKSTART_ROUTINE StartRoutine, PVOID StartContext) {
	// Temp sleep to ensure XBE gets fully loaded before scan
	XSleep(3000);
	
	// Step 1 & 2
	XbSDBLibraryHeader lib_header;
	XbSDBSectionHeader sec_header;
	
	lib_header.count = XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, NULL);
	sec_header.count = XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, NULL, true);
	
	lib_header.filters = malloc(lib_header.count * sizeof(XbSDBLibrary));
	sec_header.filters = malloc(sec_header.count * sizeof(XbSDBSection));
	
	XbSymbolDatabase_GenerateLibraryFilter((PVOID)0x00010000, &lib_header);
	XbSymbolDatabase_GenerateSectionFilter((PVOID)0x00010000, &sec_header, true);
	
	// Step 3
	uint32_t thunk_addr = XbSymbolDatabase_GetKernelThunkAddress((PVOID)0x00010000);
	// Step 4a
	XbSymbolContextHandle xapi_handle;
	
	if(!XbSymbolDatabase_CreateXbSymbolContext(&xapi_handle, scanned_func, lib_header, sec_header, thunk_addr)) {
		XReboot();
	}
	// Step 4b
	XbSymbolContext_RegisterLibrary(xapi_handle, XbSymbolLib_XAPILIB);
	// Step 5
	XbSymbolContext_ScanManual(xapi_handle);
	// Step 6
	xrefs_cnt = XbSymbolContext_ScanLibrary(xapi_handle, lib_header.filters, true);
	// Step 7
	XbSymbolContext_RegisterXRefs(xapi_handle);
	// Step 8
	XbSymbolContext_Release(xapi_handle);
	
	free(sec_header.filters);
	free(lib_header.filters);
	
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
	
	DmRegisterCommandProcessor("SYMBOLZ", symbolz);
	
	HANDLE handle, id;
	NTSTATUS status = PsCreateSystemThreadEx(&handle, 0, 8192, 0, &id, (PKSTART_ROUTINE)NULL, (PVOID)NULL, FALSE, FALSE, cheat_thread);
	
	*pfUnload = FALSE;
}
