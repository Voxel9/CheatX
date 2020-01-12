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

void DxtEntry(ULONG *pfUnload) {
	DmRegisterCommandProcessor("DUMPMEM", dump_memory);
	DmRegisterCommandProcessor("POKEMEM", poke_memory);
}
