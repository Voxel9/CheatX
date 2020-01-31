#ifndef __MEMFUNCS_H__
#define __MEMFUNCS_H__

#include "main.h"

struct freeze_entry {
	DWORD physical_addr;
	PVOID mapped_addr;
	DWORD val;
};

extern struct freeze_entry *freeze_entries;
extern int freeze_entry_cnt;

HRESULT __stdcall dump_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall poke_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall freeze_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);

#endif
