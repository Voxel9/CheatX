#include "searchfuncs.h"

int search_step = 0;
int entries_cnt = 0;

HRESULT __stdcall start_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
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

HRESULT __stdcall continue_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
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
