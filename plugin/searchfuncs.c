#include "searchfuncs.h"

int search_step = 0;
int entries_cnt = 0;

int val_size = 4;
bool is_float = false;

typedef struct {
	const char* type;
	bool (*function)(DWORD lhs, DWORD rhs);
} Comparison;

static bool is_equal(DWORD a, DWORD b) {
	if(is_float)
		return *((float*)&a) == *((float*)&b);
	
	return a == b;
}

static bool is_not_equal(DWORD a, DWORD b) {
	if(is_float)
		return *((float*)&a) != *((float*)&b);
	
	return a != b;
}

static bool is_greater(DWORD a, DWORD b) {
	if(is_float)
		return *((float*)&a) > *((float*)&b);
	
	return a > b;
}

static bool is_less(DWORD a, DWORD b) {
	if(is_float)
		return *((float*)&a) < *((float*)&b);
	
	return a < b;
}

static bool is_unknown(DWORD a, DWORD b) { return true; }

Comparison start_comparisons[] = {
	{ "equals", is_equal },
	{ "not-equals", is_not_equal },
	{ "greater-than", is_greater },
	{ "less-than", is_less },
	{ "unknown", is_unknown },
};

Comparison cont_comparisons[] = {
	{ "equals", is_equal },
	{ "not-equals", is_not_equal },
	{ "greater-than", is_greater },
	{ "less-than", is_less },
};

Comparison cont_unk_comparisons[] = {
	{ "same", is_equal },
	{ "different", is_not_equal },
	{ "greater", is_greater },
	{ "less", is_less },
};

bool compare(Comparison *comparisons, BYTE struct_cnt, const char* type, DWORD a, DWORD b) {
	for(int i = 0; i < struct_cnt; i++) {
		Comparison* comparison = &comparisons[i];
		if (strcmp(type, comparison->type) == 0) {
			return comparison->function(a, b);
		}
	}
	return false;
}

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
	
	for(int i = 0; i < PHYSICAL_ADDR_SIZE; i += val_size) {
		if(val_size == 1 && compare(start_comparisons, 5, condition, *(BYTE*)(addr+i), (BYTE)val)) {
			fwrite(&i, sizeof(DWORD), 1, fp);
			
			DWORD write_val = *(BYTE*)(addr+i);
			fwrite(&write_val, sizeof(DWORD), 1, fp);
			
			entries_cnt++;
		}
		else if(val_size == 2 && compare(start_comparisons, 5, condition, *(WORD*)(addr+i), (WORD)val)) {
			fwrite(&i, sizeof(DWORD), 1, fp);
			
			DWORD write_val = *(WORD*)(addr+i);
			fwrite(&write_val, sizeof(DWORD), 1, fp);
			
			entries_cnt++;
		}
		else if(val_size == 4 && compare(start_comparisons, 5, condition, *(DWORD*)(addr+i), val)) {
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
	
	for(int i = 0; i < entries_cnt; i++) {
		DWORD prev_addr, prev_val;
		
		fread(&prev_addr, sizeof(DWORD), 1, fprevscan);
		fread(&prev_val, sizeof(DWORD), 1, fprevscan);
		
		if(val_size == 1) {
			DWORD cur_val = *(BYTE*)(addr + prev_addr);
			
			if(compare(cont_comparisons, 4, condition, cur_val, (BYTE)val) || compare(cont_unk_comparisons, 4, condition, cur_val, prev_val)) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				
				DWORD write_val = *(BYTE*)(addr + prev_addr);
				fwrite(&write_val, sizeof(DWORD), 1, fp);
				
				new_entries_cnt++;
			}
		}
		else if(val_size == 2) {
			DWORD cur_val = *(WORD*)(addr + prev_addr);
			
			if(compare(cont_comparisons, 4, condition, cur_val, (WORD)val) || compare(cont_unk_comparisons, 4, condition, cur_val, prev_val)) {
				fwrite(&prev_addr, sizeof(DWORD), 1, fp);
				
				DWORD write_val = *(WORD*)(addr + prev_addr);
				fwrite(&write_val, sizeof(DWORD), 1, fp);
				
				new_entries_cnt++;
			}
		}
		else if(val_size == 4) {
			DWORD cur_val = *(DWORD*)(addr + prev_addr);
			
			if(compare(cont_comparisons, 4, condition, cur_val, val) || compare(cont_unk_comparisons, 4, condition, cur_val, prev_val)) {
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

HRESULT __stdcall change_type(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc) {
	char type[16];
	sscanf(szCommand, "changetype! %s", type);
	
	is_float = false;
	
	if(strcmp(type, "byte") == 0) {
		val_size = 1;
	}
	else if(strcmp(type, "word") == 0) {
		val_size = 2;
	}
	else if(strcmp(type, "dword") == 0) {
		val_size = 4;
	}
	else if(strcmp(type, "float") == 0) {
		val_size = 4;
		is_float = true;
	}
	
	return XBDM_NOERR;
}
