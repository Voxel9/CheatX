#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xboxkrnl/xboxkrnl.h>
#include <winapi/winbase.h>
#include <xbdm/xbdm.h>

int main() { (void)KeTickCount; return 0; }

static HRESULT __stdcall dump_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\memdump.bin", "wb");
	
	int physical_addr = 0x00000000;
	int size = 0x03FFFFFF;
	
	PVOID addr = MmMapIoSpace(physical_addr, size, PAGE_READWRITE);
	
	fwrite(addr, size, 1, fp);
	
	MmUnmapIoSpace(addr, size);
	
	fclose(fp);
	
	return XBDM_NOERR;
}

static HRESULT __stdcall poke_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char code_address_buf[16], val_buf[16];
	DWORD code_address, val;
	
	sscanf(szCommand, "pokemem! %s %s", code_address_buf, val_buf);
	
	code_address = strtol(code_address_buf, NULL, 16);
	val = strtol(val_buf, NULL, 16);
	
	PVOID address = MmMapIoSpace(code_address, 4, PAGE_READWRITE);
	*(DWORD*)address = val;
	
	MmUnmapIoSpace(address, 4);
	
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
	
	remove("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\search0.bin");
	FILE *fp = fopen("\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\search0.bin", "wb");
	
	search_step = 0;
	entries_cnt = 0;
	
	int physical_addr = 0x00000000;
	int size = 0x03FFFFFF;
	
	PVOID addr = MmMapIoSpace(physical_addr, size, PAGE_READWRITE);
	
	if(strcmp(condition, "equals") == 0) {
		for(DWORD i = 0; i < size; i += 4) {
			if(*(DWORD*)(addr+i) == val) {
				fwrite(&i, sizeof(DWORD), 1, fp);
				fwrite((DWORD*)(addr+i), sizeof(DWORD), 1, fp);
				entries_cnt++;
			}
		}
	}
	
	sprintf(szResp, "%d entries found.", entries_cnt);
	
	MmUnmapIoSpace(addr, size);
	
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
	remove(prevpath);
	FILE *fp = fopen(fpath, "wb");
	
	int physical_addr = 0x00000000;
	int size = 0x03FFFFFF;
	
	PVOID addr = MmMapIoSpace(physical_addr, size, PAGE_READWRITE);
	
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
	
	entries_cnt = new_entries_cnt;
	
	sprintf(szResp, "%d entries found.", new_entries_cnt);
	
	MmUnmapIoSpace(addr, size);
	
	fclose(fp);
	fclose(fprevscan);
	
	search_step++;
	
	return XBDM_NOERR;
}

static VOID NTAPI cheat_thread(PKSTART_ROUTINE StartRoutine, PVOID StartContext) {
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
