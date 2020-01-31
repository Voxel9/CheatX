#ifndef __SEARCHFUNCS_H__
#define __SEARCHFUNCS_H__

#include "main.h"

HRESULT __stdcall start_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);
HRESULT __stdcall continue_search(LPCSTR szCommand, LPSTR szResp, DWORD cchResp, PDM_CMDCONT pdmcc);

#endif
