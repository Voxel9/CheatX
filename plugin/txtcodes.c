#include "txtcodes.h"
#include "memfuncs.h"

void LoadCodesFromFile() {
	// Assuming XBE was loaded at 0x10000
	uint32_t cert_addr = *(DWORD*)0x00010118;
	uint32_t title_id = *(DWORD*)(cert_addr + 0x8);
	
	char filebuf[256];
	sprintf(filebuf, "\\Device\\Harddisk0\\Partition1\\DEVKIT\\dxt\\CheatX\\txtcodes\\%08X.txt", title_id);
	
	FILE *fp = fopen(filebuf, "r");
	
	if(fp) {
		char linebuf[256];
		
		while(fgets(linebuf, 256, fp)) {
			switch(linebuf[0]) {
				case '0': {
					DWORD address, val;
					sscanf(linebuf, "%x %x", &address, &val);
					
					add_freeze_entry(address, val);
				} break;
			}
		}
		
		fclose(fp);
	}
}
