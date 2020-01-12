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
	DWORD address;
	DWORD val;
};

struct freeze_entry *freeze_entries;
int freeze_entry_cnt = 0;

static HRESULT __stdcall freeze_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char code_address_buf[16], val_buf[16];
	DWORD code_address, val;
	
	sscanf(szCommand, "freezemem! %s %s", code_address_buf, val_buf);
	
	code_address = strtol(code_address_buf, NULL, 16);
	val = strtol(val_buf, NULL, 16);
	
	freeze_entry_cnt++;
	freeze_entries = realloc(freeze_entries, sizeof(struct freeze_entry) * freeze_entry_cnt);
	
	freeze_entries[freeze_entry_cnt-1].address = code_address;
	freeze_entries[freeze_entry_cnt-1].val = val;
	
	return XBDM_NOERR;
}

static VOID NTAPI cheat_thread(PKSTART_ROUTINE StartRoutine, PVOID StartContext) {
	while(1) {
		XSleep(100);
		
		for(int i = 0; i < freeze_entry_cnt; i++) {
			PVOID address = MmMapIoSpace(freeze_entries[i].address, 4, PAGE_READWRITE);
			
			*(DWORD*)address = freeze_entries[i].val;
			
			MmUnmapIoSpace(address, 4);
		}
	}
}

void DxtEntry(ULONG *pfUnload) {
	DmRegisterCommandProcessor("DUMPMEM", dump_memory);
	DmRegisterCommandProcessor("POKEMEM", poke_memory);
	DmRegisterCommandProcessor("FREEZEMEM", freeze_memory);
	
	HANDLE handle, id;
	NTSTATUS status = PsCreateSystemThreadEx(&handle, 0, 8192, 0, &id, (PKSTART_ROUTINE)NULL, (PVOID)NULL, FALSE, FALSE, cheat_thread);
	
	*pfUnload = FALSE;
}
