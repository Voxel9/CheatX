#ifndef __MEMFUNCS_H__
#define __MEMFUNCS_H__

#include "main.h"

struct freeze_entry {
    DWORD physical_addr;
    PVOID mapped_addr;
    DWORD val;
};

void add_freeze_entry(DWORD address, DWORD val);
void apply_freeze_entries();

HRESULT __stdcall dump_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall peek_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall poke_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall freeze_memory(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);

#endif
