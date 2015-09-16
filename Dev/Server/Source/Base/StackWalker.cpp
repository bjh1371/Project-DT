////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file StackWorker.cpp
/// \author bjh1371
/// \date 2015/06/08
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**********************************************************************
 * 
 * StackWalker.cpp
 *
 *
 * History:
 *  2005-07-27   v1    - First public release on http://www.codeproject.com/
 *                       http://www.codeproject.com/threads/StackWalker.asp
 *  2005-07-28   v2    - Changed the params of the constructor and ShowCallstack
 *                       (to simplify the usage)
 *  2005-08-01   v3    - Changed to use 'CONTEXT_FULL' instead of CONTEXT_ALL 
 *                       (should also be enough)
 *                     - Changed to compile correctly with the PSDK of VC7.0
 *                       (GetFileVersionInfoSizeA and GetFileVersionInfoA is wrongly defined:
 *                        it uses LPSTR instead of LPCSTR as first paremeter)
 *                     - Added declarations to support VC5/6 without using 'dbghelp.h'
 *                     - Added a 'pUserData' member to the ShowCallstack function and the 
 *                       PReadProcessMemoryRoutine declaration (to pass some user-defined data, 
 *                       which can be used in the readMemoryFunction-callback)
 *  2005-08-02   v4    - OnSymInit now also outputs the OS-Version by default
 *                     - Added example for doing an exception-callstack-walking in main.cpp
 *                       (thanks to owillebo: http://www.codeproject.com/script/profile/whos_who.asp?id=536268)
 *  2005-08-05   v5    - Removed most Lint (http://www.gimpel.com/) errors... thanks to Okko Willeboordse!
 *  2008-08-04   v6    - Fixed Bug: Missing LEAK-end-tag
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=2502890#xx2502890xx
 *                       Fixed Bug: Compiled with "WIN32_LEAN_AND_MEAN"
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=1824718#xx1824718xx
 *                       Fixed Bug: Compiling with "/Wall"
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=2638243#xx2638243xx
 *                       Fixed Bug: Now checking SymUseSymSrv
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1388979#xx1388979xx
 *                       Fixed Bug: Support for recursive function calls
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1434538#xx1434538xx
 *                       Fixed Bug: Missing FreeLibrary call in "GetModuleListTH32"
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=1326923#xx1326923xx
 *                       Fixed Bug: SymDia is number 7, not 9!
 *  2008-09-11   v7      For some (undocumented) reason, dbhelp.h is needing a packing of 8!
 *                       Thanks to Teajay which reported the bug...
 *                       http://www.codeproject.com/KB/applications/leakfinder.aspx?msg=2718933#xx2718933xx
 *  2008-11-27   v8      Debugging Tools for Windows are now stored in a different directory
 *                       Thanks to Luiz Salamon which reported this "bug"...
 *                       http://www.codeproject.com/KB/threads/StackWalker.aspx?msg=2822736#xx2822736xx
 *  2009-04-10   v9      License slihtly corrected (<ORGANIZATION> replaced)
 *  2009-11-01-  v10     Moved to stackwalker.codeplex.com
 *
 * LICENSE (http://www.opensource.org/licenses/bsd-license.php)
 *
 *   Copyright (c) 2005-2009, Jochen Kalmbach
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without modification, 
 *   are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer. 
 *   Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution. 
 *   Neither the name of Jochen Kalmbach nor the names of its contributors may be 
 *   used to endorse or promote products derived from this software without 
 *   specific prior written permission. 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE 
 *   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 *   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND 
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************/
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#pragma comment(lib, "version.lib")  // for "VerQueryValue"
#pragma warning(disable:4826)

#include "StackWalker.h"

// 유니코드
#define DBGHELP_TRANSLATE_TCHAR

#include <dbghelp.h>
#include <tlhelp32.h>

// 유니코드
#if defined(UNICODE) | defined(_UNICODE)
#define IMAGEHLP_SYMBOL64 IMAGEHLP_SYMBOLW64
#define PIMAGEHLP_SYMBOL64 PIMAGEHLP_SYMBOLW64
#define PSYMBOL_INFO PSYMBOL_INFOW
#define SYMBOL_INFO  SYMBOL_INFOW
#endif
// Some missing defines (for VC5/6):
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif  


// secure-CRT_functions are only available starting with VC8
#if _MSC_VER < 1400
#define strcpy_s strcpy
#define strcat_s(dst, len, src) strcat(dst, src)
#define _snprintf_s _snprintf
#define _tcscat_s _tcscat
#endif

// Normally it should be enough to use 'CONTEXT_FULL' (better would be 'CONTEXT_ALL')
#define USED_CONTEXT_FLAGS CONTEXT_FULL


class CStackWalkerInternal
{
public:
  CStackWalkerInternal(CStackWalker *parent, HANDLE hProcess)
  {
    m_parent = parent;
    m_hDbhHelp = NULL;
    pSC = NULL;
    m_hProcess = hProcess;
    m_szSymPath = NULL;
    pSFTA = NULL;
    pSGLFA = NULL;
    pSGMB = NULL;
    pSGMI = NULL;
    pSGO = NULL;
    pSGSFA = NULL;
    pSI = NULL;
    pSLM = NULL;
    pSSO = NULL;
    pSW = NULL;
    pUDSN = NULL;
    pSGSP = NULL;
  }
  ~CStackWalkerInternal()
  {
    if (pSC != NULL)
      pSC(m_hProcess);  // SymCleanup
    if (m_hDbhHelp != NULL)
      FreeLibrary(m_hDbhHelp);
    m_hDbhHelp = NULL;
    m_parent = NULL;
    if(m_szSymPath != NULL)
      free(m_szSymPath);
    m_szSymPath = NULL;
  }
  BOOL Init(LPCTSTR szSymPath)
  {
    if (m_parent == NULL)
      return FALSE;
    // Dynamically load the Entry-Points for dbghelp.dll:
    // First try to load the newsest one from
    TCHAR szTemp[4096];
    // But before wqe do this, we first check if the ".local" file exists
    if (GetModuleFileName(NULL, szTemp, 4096) > 0)
    {
      _tcscat_s(szTemp, _T(".local"));
      if (GetFileAttributes(szTemp) == INVALID_FILE_ATTRIBUTES)
      {
        // ".local" file does not exist, so we can try to load the dbghelp.dll from the "Debugging Tools for Windows"
        // Ok, first try the new path according to the archtitecture:
#ifdef _M_IX86
        if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
        {
          _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (x86)\\dbghelp.dll"));
          // now check if the file exists:
          if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
          {
            m_hDbhHelp = LoadLibrary(szTemp);
          }
        }
#elif _M_X64
        if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
        {
          _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (x64)\\dbghelp.dll"));
          // now check if the file exists:
          if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
          {
            m_hDbhHelp = LoadLibrary(szTemp);
          }
        }
#elif _M_IA64
        if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
        {
          _tcscat_s(szTemp, _T("\\Debugging Tools for Windows (ia64)\\dbghelp.dll"));
          // now check if the file exists:
          if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
          {
            m_hDbhHelp = LoadLibrary(szTemp);
          }
        }
#endif
        // If still not found, try the old directories...
        if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
        {
          _tcscat_s(szTemp, _T("\\Debugging Tools for Windows\\dbghelp.dll"));
          // now check if the file exists:
          if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
          {
            m_hDbhHelp = LoadLibrary(szTemp);
          }
        }
#if defined _M_X64 || defined _M_IA64
        // Still not found? Then try to load the (old) 64-Bit version:
        if ( (m_hDbhHelp == NULL) && (GetEnvironmentVariable(_T("ProgramFiles"), szTemp, 4096) > 0) )
        {
          _tcscat_s(szTemp, _T("\\Debugging Tools for Windows 64-Bit\\dbghelp.dll"));
          if (GetFileAttributes(szTemp) != INVALID_FILE_ATTRIBUTES)
          {
            m_hDbhHelp = LoadLibrary(szTemp);
          }
        }
#endif
      }
    }
    if (m_hDbhHelp == NULL)  // if not already loaded, try to load a default-one
      m_hDbhHelp = LoadLibrary( _T("dbghelp.dll") );
    if (m_hDbhHelp == NULL)
      return FALSE;

	// 유니코드는 유니코드로
#if defined(UNICODE) | defined(_UNICODE)
	pSI = (tSI)GetProcAddress(m_hDbhHelp, "SymInitializeW");
	pSGLFA = (tSGLFA)GetProcAddress(m_hDbhHelp, "SymGetLineFromAddrW64");
	pSGMI = (tSGMI)GetProcAddress(m_hDbhHelp, "SymGetModuleInfoW64");
	pUDSN = (tUDSN)GetProcAddress(m_hDbhHelp, "UnDecorateSymbolNameW");;
	pSLM = (tSLM)GetProcAddress(m_hDbhHelp, "SymLoadModuleExW");
	pSGSP = (tSGSP)GetProcAddress(m_hDbhHelp, "SymGetSearchPathW");
	pSGSFA = (tSGSFA)GetProcAddress(m_hDbhHelp, "SymFromAddrW");
#else
	pSI = (tSI) GetProcAddress(m_hDbhHelp, "SymInitialize" );
	pSGLFA = (tSGLFA)GetProcAddress(m_hDbhHelp, "SymGetLineFromAddr64");
	pSGMI = (tSGMI)GetProcAddress(m_hDbhHelp, "SymGetModuleInfo64");
	pUDSN = (tUDSN)GetProcAddress(m_hDbhHelp, "UnDecorateSymbolName");;
	pSLM = (tSLM)GetProcAddress(m_hDbhHelp, "SymLoadModuleEx");
	pSGSP = (tSGSP)GetProcAddress(m_hDbhHelp, "SymGetSearchPath");
	pSGSFA = (tSGSFA)GetProcAddress(m_hDbhHelp, "SymFromAddr");
#endif
	
	// 기타
	pSC = (tSC)GetProcAddress(m_hDbhHelp, "SymCleanup");
	pSW = (tSW)GetProcAddress(m_hDbhHelp, "StackWalk64");
	pSGO = (tSGO)GetProcAddress(m_hDbhHelp, "SymGetOptions");
	pSSO = (tSSO)GetProcAddress(m_hDbhHelp, "SymSetOptions");
	pSFTA = (tSFTA)GetProcAddress(m_hDbhHelp, "SymFunctionTableAccess64");	
	pSGMB = (tSGMB)GetProcAddress(m_hDbhHelp, "SymGetModuleBase64");

    if ( pSC == NULL || pSFTA == NULL || pSGMB == NULL || pSGMI == NULL ||
      pSGO == NULL || pSGSFA == NULL || pSI == NULL || pSSO == NULL ||
      pSW == NULL || pUDSN == NULL || pSLM == NULL )
    {
      FreeLibrary(m_hDbhHelp);
      m_hDbhHelp = NULL;
      pSC = NULL;
      return FALSE;
    }

    // SymInitialize
    if (szSymPath != NULL)
      m_szSymPath = _wcsdup(szSymPath);
    if (this->pSI(m_hProcess, m_szSymPath, FALSE) == FALSE)
      this->m_parent->OnDbgHelpErr(_T("SymInitialize"), GetLastError(), 0);
      
    DWORD symOptions = this->pSGO();  // SymGetOptions
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
    //symOptions |= SYMOPT_NO_PROMPTS;
    // SymSetOptions
    symOptions = this->pSSO(symOptions);

    TCHAR buf[CStackWalker::STACKWALK_MAX_NAMELEN] = {0};
    if (this->pSGSP != NULL)
    {
      if (this->pSGSP(m_hProcess, buf, CStackWalker::STACKWALK_MAX_NAMELEN) == FALSE)
        this->m_parent->OnDbgHelpErr(_T("SymGetSearchPath"), GetLastError(), 0);
    }
    TCHAR szUserName[1024] = {0};
    DWORD dwSize = 1024;
    GetUserName(szUserName, &dwSize);
    this->m_parent->OnSymInit(buf, symOptions, szUserName);

    return TRUE;
  }

  CStackWalker *m_parent;

  HMODULE m_hDbhHelp;
  HANDLE m_hProcess;
  LPTSTR m_szSymPath;

  // SymCleanup()
  typedef BOOL (__stdcall *tSC)( IN HANDLE hProcess );
  tSC pSC;

  // SymFunctionTableAccess64()
  typedef PVOID (__stdcall *tSFTA)( HANDLE hProcess, DWORD64 AddrBase );
  tSFTA pSFTA;

  // SymGetLineFromAddr64()
  typedef BOOL (__stdcall *tSGLFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
    OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line );
  tSGLFA pSGLFA;

  // SymGetModuleBase64()
  typedef DWORD64 (__stdcall *tSGMB)( IN HANDLE hProcess, IN DWORD64 dwAddr );
  tSGMB pSGMB;

  // SymGetModuleInfo64()
  typedef BOOL (__stdcall *tSGMI)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT IMAGEHLP_MODULE64* ModuleInfo );
  tSGMI pSGMI;

//  // SymGetModuleInfo64()
//  typedef BOOL (__stdcall *tSGMI_V3)( IN HANDLE hProcess, IN DWORD64 dwAddr, OUT IMAGEHLP_MODULE64_V3 *ModuleInfo );
//  tSGMI_V3 pSGMI_V3;

  // SymGetOptions()
  typedef DWORD (__stdcall *tSGO)( VOID );
  tSGO pSGO;

  // SymGetSymFromAddr64()
  typedef BOOL (__stdcall *tSGSFA)( IN HANDLE hProcess, IN DWORD64 dwAddr,
    OUT PDWORD64 pdwDisplacement, OUT PSYMBOL_INFO Symbol );
  tSGSFA pSGSFA;

  // SymInitialize()
  typedef BOOL (__stdcall *tSI)( IN HANDLE hProcess, IN PTSTR UserSearchPath, IN BOOL fInvadeProcess );
  tSI pSI;

  // SymLoadModuleEx()
  typedef DWORD64(__stdcall *tSLM)(IN HANDLE hProcess, IN HANDLE hFile,
	IN PTSTR ImageName, IN PTSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD DllSize, IN PMODLOAD_DATA Data, IN DWORD Flags);
  tSLM pSLM;

  // SymSetOptions()
  typedef DWORD (__stdcall *tSSO)( IN DWORD SymOptions );
  tSSO pSSO;

  // StackWalk64()
  typedef BOOL (__stdcall *tSW)( 
    DWORD MachineType, 
    HANDLE hProcess,
    HANDLE hThread, 
    LPSTACKFRAME64 StackFrame, 
    PVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress );
  tSW pSW;

  // UnDecorateSymbolName()
  typedef DWORD (__stdcall WINAPI *tUDSN)( PCTSTR DecoratedName, PTSTR UnDecoratedName,
    DWORD UndecoratedLength, DWORD Flags );
  tUDSN pUDSN;

  typedef BOOL (__stdcall WINAPI *tSGSP)(HANDLE hProcess, PTSTR SearchPath, DWORD SearchPathLength);
  tSGSP pSGSP;


private:

  BOOL GetModuleListTH32(HANDLE hProcess, DWORD pid)
  {
    // CreateToolhelp32Snapshot()
    typedef HANDLE (__stdcall *tCT32S)(DWORD dwFlags, DWORD th32ProcessID);
    // Module32First()
    typedef BOOL (__stdcall *tM32F)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
    // Module32Next()
    typedef BOOL (__stdcall *tM32N)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

    // try both dlls...
    const TCHAR *dllname[] = { _T("kernel32.dll"), _T("tlhelp32.dll") };
    HINSTANCE hToolhelp = NULL;
    tCT32S pCT32S = NULL;
    tM32F pM32F = NULL;
    tM32N pM32N = NULL;

    HANDLE hSnap;
    MODULEENTRY32 me;
    me.dwSize = sizeof(me);
    BOOL keepGoing;
    size_t i;

    for (i = 0; i<(sizeof(dllname) / sizeof(dllname[0])); i++ )
    {
      hToolhelp = LoadLibrary( dllname[i] );
      if (hToolhelp == NULL)
        continue;

// 유니코드
#if defined(UNICODE) | defined(_UNICODE)
	  pM32F = (tM32F)GetProcAddress(hToolhelp, "Module32FirstW");
	  pM32N = (tM32N)GetProcAddress(hToolhelp, "Module32NextW");
#else
	  pM32F = (tM32F) GetProcAddress(hToolhelp, "Module32First");
	  pM32N = (tM32N)GetProcAddress(hToolhelp, "Module32Next");
#endif
      
	  pCT32S = (tCT32S) GetProcAddress(hToolhelp, "CreateToolhelp32Snapshot");
      
      if ( (pCT32S != NULL) && (pM32F != NULL) && (pM32N != NULL) )
        break; // found the functions!
      FreeLibrary(hToolhelp);
      hToolhelp = NULL;
    }

    if (hToolhelp == NULL)
      return FALSE;

    hSnap = pCT32S( TH32CS_SNAPMODULE, pid );
    if (hSnap == (HANDLE) -1)
    {
      FreeLibrary(hToolhelp);
      return FALSE;
    }

    keepGoing = !!pM32F( hSnap, &me );
    int cnt = 0;
    while (keepGoing)
    {
      this->LoadModule(hProcess, me.szExePath, me.szModule, (DWORD64) me.modBaseAddr, me.modBaseSize);
      cnt++;
      keepGoing = !!pM32N( hSnap, &me );
    }
    CloseHandle(hSnap);
    FreeLibrary(hToolhelp);
    if (cnt <= 0)
      return FALSE;
    return TRUE;
  }  // GetModuleListTH32

  // **************************************** PSAPI ************************
  typedef struct _MODULEINFO {
      LPVOID lpBaseOfDll;
      DWORD SizeOfImage;
      LPVOID EntryPoint;
  } MODULEINFO, *LPMODULEINFO;

  BOOL GetModuleListPSAPI(HANDLE hProcess)
  {
    // EnumProcessModules()
    typedef BOOL (__stdcall *tEPM)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded );
    // GetModuleFileNameEx()
    typedef DWORD (__stdcall *tGMFNE)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize );
    // GetModuleBaseName()
    typedef DWORD (__stdcall *tGMBN)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize );
    // GetModuleInformation()
    typedef BOOL (__stdcall *tGMI)(HANDLE hProcess, HMODULE hModule, LPMODULEINFO pmi, DWORD nSize );

    HINSTANCE hPsapi;
    tEPM pEPM;
    tGMFNE pGMFNE;
    tGMBN pGMBN;
    tGMI pGMI;

    DWORD i;
    //ModuleEntry e;
    DWORD cbNeeded;
    MODULEINFO mi;
    HMODULE *hMods = 0;
    TCHAR *tt = NULL;
    TCHAR *tt2 = NULL;
    const SIZE_T TTBUFLEN = 8096;
    int cnt = 0;

    hPsapi = LoadLibrary( _T("psapi.dll") );
    if (hPsapi == NULL)
      return FALSE;

    pEPM = (tEPM) GetProcAddress( hPsapi, "EnumProcessModules" );
    pGMFNE = (tGMFNE) GetProcAddress( hPsapi, "GetModuleFileNameEx" );
    pGMBN = (tGMFNE) GetProcAddress( hPsapi, "GetModuleBaseName" );
    pGMI = (tGMI) GetProcAddress( hPsapi, "GetModuleInformation" );
    if ( (pEPM == NULL) || (pGMFNE == NULL) || (pGMBN == NULL) || (pGMI == NULL) )
    {
      // we couldn큧 find all functions
      FreeLibrary(hPsapi);
      return FALSE;
    }

    hMods = (HMODULE*) malloc(sizeof(HMODULE) * (TTBUFLEN / sizeof HMODULE));
    tt = (TCHAR*) malloc(sizeof(TCHAR) * TTBUFLEN);
    tt2 = (TCHAR*) malloc(sizeof(TCHAR) * TTBUFLEN);
    if ( (hMods == NULL) || (tt == NULL) || (tt2 == NULL) )
      goto cleanup;

    if ( ! pEPM( hProcess, hMods, TTBUFLEN, &cbNeeded ) )
    {
      //_ftprintf(fLogFile, _T("%lu: EPM failed, GetLastError = %lu\n"), g_dwShowCount, gle );
      goto cleanup;
    }

    if ( cbNeeded > TTBUFLEN )
    {
      //_ftprintf(fLogFile, _T("%lu: More than %lu module handles. Huh?\n"), g_dwShowCount, lenof( hMods ) );
      goto cleanup;
    }

    for ( i = 0; i < cbNeeded / sizeof hMods[0]; i++ )
    {
      // base address, size
      pGMI(hProcess, hMods[i], &mi, sizeof mi );
      // image file name
      tt[0] = 0;
      pGMFNE(hProcess, hMods[i], tt, TTBUFLEN );
      // module name
      tt2[0] = 0;
      pGMBN(hProcess, hMods[i], tt2, TTBUFLEN );

      DWORD dwRes = this->LoadModule(hProcess, tt, tt2, (DWORD64) mi.lpBaseOfDll, mi.SizeOfImage);
      if (dwRes != ERROR_SUCCESS)
        this->m_parent->OnDbgHelpErr(_T("LoadModule"), dwRes, 0);
      cnt++;
    }

  cleanup:
    if (hPsapi != NULL) FreeLibrary(hPsapi);
    if (tt2 != NULL) free(tt2);
    if (tt != NULL) free(tt);
    if (hMods != NULL) free(hMods);

    return cnt != 0;
  }  // GetModuleListPSAPI

  DWORD LoadModule(HANDLE hProcess, LPCTSTR img, LPCTSTR mod, DWORD64 baseAddr, DWORD size)
  {
    TCHAR *szImg = _wcsdup(img);
    TCHAR *szMod = _wcsdup(mod);
    DWORD result = ERROR_SUCCESS;

	PMODLOAD_DATA data;
	ZeroMemory(&data, sizeof(data));
	DWORD flags = 0;

    if ( (szImg == NULL) || (szMod == NULL) )
      result = ERROR_NOT_ENOUGH_MEMORY;
    else
    {
      if (pSLM(hProcess, 0, szImg, szMod, baseAddr, size, data, flags) == 0)
        result = GetLastError();
    }
    Milli_t fileVersion = 0;
    if ( (m_parent != NULL) && (szImg != NULL) )
    {
      // try to retrive the file-version:
      if ( (this->m_parent->m_options & CStackWalker::RetrieveFileVersion) != 0)
      {
        VS_FIXEDFILEINFO *fInfo = NULL;
        DWORD dwHandle;
        DWORD dwSize = GetFileVersionInfoSize(szImg, &dwHandle);
        if (dwSize > 0)
        {
          LPVOID vData = malloc(dwSize);
          if (vData != NULL)
          {
            if (GetFileVersionInfo(szImg, dwHandle, dwSize, vData) != 0)
            {
              UINT len;
              TCHAR szSubBlock[] = _T("\\");
              if (VerQueryValue(vData, szSubBlock, (LPVOID*) &fInfo, &len) == 0)
                fInfo = NULL;
              else
              {
                fileVersion = ((ULONGLONG)fInfo->dwFileVersionLS) + ((ULONGLONG)fInfo->dwFileVersionMS << 32);
              }
            }
            free(vData);
          }
        }
      }

      // Retrive some additional-infos about the module
      IMAGEHLP_MODULE64 Module;
	  memset(&Module, 0, sizeof(Module));

      const TCHAR *szSymType = _T("-unknown-");
      if (this->GetModuleInfo(hProcess, baseAddr, &Module) != FALSE)
      {
        switch(Module.SymType)
        {
          case SymNone:
            szSymType = _T("-nosymbols-");
            break;
          case SymCoff:  // 1
            szSymType = _T("COFF");
            break;
          case SymCv:  // 2
            szSymType = _T("CV");
            break;
          case SymPdb:  // 3
            szSymType = _T("PDB");
            break;
          case SymExport:  // 4
            szSymType = _T("-exported-");
            break;
          case SymDeferred:  // 5
            szSymType = _T("-deferred-");
            break;
          case SymSym:  // 6
            szSymType = _T("SYM");
            break;
          case 7: // SymDia:
            szSymType = _T("DIA");
            break;
          case 8: //SymVirtual:
            szSymType = _T("Virtual");
            break;
        }
      }
      this->m_parent->OnLoadModule(img, mod, baseAddr, size, result, szSymType, Module.LoadedImageName, fileVersion);
    }
    if (szImg != NULL) free(szImg);
    if (szMod != NULL) free(szMod);
    return result;
  }
public:
  BOOL LoadModules(HANDLE hProcess, DWORD dwProcessId)
  {
    // first try toolhelp32
    if (GetModuleListTH32(hProcess, dwProcessId))
      return true;
    // then try psapi
    return GetModuleListPSAPI(hProcess);
  }


  BOOL GetModuleInfo(HANDLE hProcess, DWORD64 baseAddr, IMAGEHLP_MODULE64* pModuleInfo)
  {
    if(this->pSGMI == NULL)
    {
      SetLastError(ERROR_DLL_INIT_FAILED);
      return FALSE;
    }
    // First try to use the larger ModuleInfo-Structure
//    memset(pModuleInfo, 0, sizeof(IMAGEHLP_MODULE64_V3));
//    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64_V3);
//    if (this->pSGMI_V3 != NULL)
//    {
//      if (this->pSGMI_V3(hProcess, baseAddr, pModuleInfo) != FALSE)
//        return TRUE;
//      // check if the parameter was wrong (size is bad...)
//      if (GetLastError() != ERROR_INVALID_PARAMETER)
//        return FALSE;
//    }
    // could not retrive the bigger structure, try with the smaller one (as defined in VC7.1)...
    pModuleInfo->SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

    if (this->pSGMI(hProcess, baseAddr, pModuleInfo) != FALSE)
    {
      return TRUE;
    }

    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }
};

// #############################################################
CStackWalker::CStackWalker(DWORD dwProcessId, HANDLE hProcess)
{
  this->m_options = OptionsAll;
  this->m_modulesLoaded = FALSE;
  this->m_hProcess = hProcess;
  this->m_sw = new CStackWalkerInternal(this, this->m_hProcess);
  this->m_dwProcessId = dwProcessId;
  this->m_szSymPath = NULL;
  this->m_MaxRecursionCount = 1000;
}
CStackWalker::CStackWalker(int options, LPCTSTR szSymPath, DWORD dwProcessId, HANDLE hProcess)
{
  this->m_options = options;
  this->m_modulesLoaded = FALSE;
  this->m_hProcess = hProcess;
  this->m_sw = new CStackWalkerInternal(this, this->m_hProcess);
  this->m_dwProcessId = dwProcessId;
  if (szSymPath != NULL)
  {
    this->m_szSymPath = _wcsdup(szSymPath);
    this->m_options |= SymBuildPath;
  }
  else
    this->m_szSymPath = NULL;
  this->m_MaxRecursionCount = 1000;
}

CStackWalker::~CStackWalker()
{
  if (m_szSymPath != NULL)
    free(m_szSymPath);
  m_szSymPath = NULL;
  if (this->m_sw != NULL)
    delete this->m_sw;
  this->m_sw = NULL;
}

BOOL CStackWalker::LoadModules()
{
  if (this->m_sw == NULL)
  {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }
  if (m_modulesLoaded != FALSE)
    return TRUE;

  // Build the sym-path:
  TCHAR *szSymPath = NULL;
  if ( (this->m_options & SymBuildPath) != 0)
  {
    const size_t nSymPathLen = 4096;
    szSymPath = (TCHAR*) malloc(sizeof(TCHAR) * nSymPathLen);
    if (szSymPath == NULL)
    {
      SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return FALSE;
    }
    szSymPath[0] = 0;
    // Now first add the (optional) provided sympath:
    if (this->m_szSymPath != NULL)
    {
      
	  wcscat_s(szSymPath, nSymPathLen, this->m_szSymPath);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
    }

    wcscat_s(szSymPath, nSymPathLen, _T(".;"));

    const size_t nTempLen = 1024;
    TCHAR szTemp[nTempLen];
    // Now add the current directory:
    if (GetCurrentDirectory(nTempLen, szTemp) > 0)
    {
      szTemp[nTempLen-1] = 0;
      wcscat_s(szSymPath, nSymPathLen, szTemp);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
    }

    // Now add the path for the main-module:
    if (GetModuleFileName(NULL, szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      for (TCHAR *p = (szTemp+wcslen(szTemp)-1); p >= szTemp; --p)
      {
        // locate the rightmost path separator
        if ( (*p == _T('\\')) || (*p == _T('/')) || (*p == _T(':')) )
        {
          *p = 0;
          break;
        }
      }  // for (search for path separator...)
      if (wcslen(szTemp) > 0)
      {
        wcscat_s(szSymPath, nSymPathLen, szTemp);
        wcscat_s(szSymPath, nSymPathLen, _T(";"));
      }
    }
    if (GetEnvironmentVariable(_T("_NT_SYMBOL_PATH"), szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      wcscat_s(szSymPath, nSymPathLen, szTemp);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
    }
    if (GetEnvironmentVariable(_T("_NT_ALTERNATE_SYMBOL_PATH"), szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      wcscat_s(szSymPath, nSymPathLen, szTemp);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
    }
    if (GetEnvironmentVariable(_T("SYSTEMROOT"), szTemp, nTempLen) > 0)
    {
      szTemp[nTempLen-1] = 0;
      wcscat_s(szSymPath, nSymPathLen, szTemp);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
      // also add the "system32"-directory:
      wcscat_s(szTemp, nTempLen, _T("\\system32"));
      wcscat_s(szSymPath, nSymPathLen, szTemp);
      wcscat_s(szSymPath, nSymPathLen, _T(";"));
    }

    if ( (this->m_options & SymUseSymSrv) != 0)
    {
      if (GetEnvironmentVariable(_T("SYSTEMDRIVE"), szTemp, nTempLen) > 0)
      {
        szTemp[nTempLen-1] = 0;
		wcscat_s(szSymPath, nSymPathLen, _T("SRV*"));
		wcscat_s(szSymPath, nSymPathLen, szTemp);
		wcscat_s(szSymPath, nSymPathLen, _T("\\websymbols"));
		wcscat_s(szSymPath, nSymPathLen, _T("*http://msdl.microsoft.com/download/symbols;"));
      }
      else
        wcscat_s(szSymPath, nSymPathLen, _T("SRV*c:\\websymbols*http://msdl.microsoft.com/download/symbols;"));
    }
  }  // if SymBuildPath

  // First Init the whole stuff...
  BOOL bRet = this->m_sw->Init(szSymPath);
  if (szSymPath != NULL) free(szSymPath); szSymPath = NULL;
  if (bRet == FALSE)
  {
    this->OnDbgHelpErr(_T("Error while initializing dbghelp.dll"), 0, 0);
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }

  bRet = this->m_sw->LoadModules(this->m_hProcess, this->m_dwProcessId);
  if (bRet != FALSE)
    m_modulesLoaded = TRUE;
  return bRet;
}


// The following is used to pass the "userData"-Pointer to the user-provided readMemoryFunction
// This has to be done due to a problem with the "hProcess"-parameter in x64...
// Because this class is in no case multi-threading-enabled (because of the limitations 
// of dbghelp.dll) it is "safe" to use a static-variable
static CStackWalker::PReadProcessMemoryRoutine s_readMemoryFunction = NULL;
static LPVOID s_readMemoryFunction_UserData = NULL;

BOOL CStackWalker::ShowCallstack(HANDLE hThread, const CONTEXT *context, PReadProcessMemoryRoutine readMemoryFunction, LPVOID pUserData)
{
  CONTEXT c;
  CallstackEntry csEntry;
  PSYMBOL_INFO pSym = NULL;
  IMAGEHLP_MODULE64 Module;
  IMAGEHLP_LINE64 Line;
  int frameNum;
  bool bLastEntryCalled = true;
  int curRecursionCount = 0;

  if (m_modulesLoaded == FALSE)
    this->LoadModules();  // ignore the result...

  if (this->m_sw->m_hDbhHelp == NULL)
  {
    SetLastError(ERROR_DLL_INIT_FAILED);
    return FALSE;
  }

  s_readMemoryFunction = readMemoryFunction;
  s_readMemoryFunction_UserData = pUserData;

  if (context == NULL)
  {
    // If no context is provided, capture the context
    if (hThread == GetCurrentThread())
    {
      GET_CURRENT_CONTEXT(c, USED_CONTEXT_FLAGS);
    }
    else
    {
      SuspendThread(hThread);
      memset(&c, 0, sizeof(CONTEXT));
      c.ContextFlags = USED_CONTEXT_FLAGS;
      if (GetThreadContext(hThread, &c) == FALSE)
      {
        ResumeThread(hThread);
        return FALSE;
      }
    }
  }
  else
    c = *context;

  // init STACKFRAME for first call
  STACKFRAME64 s; // in/out stackframe
  memset(&s, 0, sizeof(s));
  DWORD imageType;
#ifdef _M_IX86
  // normally, call ImageNtHeader() and use machine info from PE header
  imageType = IMAGE_FILE_MACHINE_I386;
  s.AddrPC.Offset = c.Eip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Ebp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Esp;
  s.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
  imageType = IMAGE_FILE_MACHINE_AMD64;
  s.AddrPC.Offset = c.Rip;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.Rsp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.Rsp;
  s.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
  imageType = IMAGE_FILE_MACHINE_IA64;
  s.AddrPC.Offset = c.StIIP;
  s.AddrPC.Mode = AddrModeFlat;
  s.AddrFrame.Offset = c.IntSp;
  s.AddrFrame.Mode = AddrModeFlat;
  s.AddrBStore.Offset = c.RsBSP;
  s.AddrBStore.Mode = AddrModeFlat;
  s.AddrStack.Offset = c.IntSp;
  s.AddrStack.Mode = AddrModeFlat;
#else
#error "Platform not supported!"
#endif

  pSym = (PSYMBOL_INFO) malloc(sizeof(SYMBOL_INFO) + STACKWALK_MAX_NAMELEN);
  if (!pSym) goto cleanup;  // not enough memory...
  memset(pSym, 0, sizeof(PSYMBOL_INFO) + STACKWALK_MAX_NAMELEN);
  pSym->SizeOfStruct = sizeof(SYMBOL_INFO);
  pSym->MaxNameLen = STACKWALK_MAX_NAMELEN;

  memset(&Line, 0, sizeof(Line));
  Line.SizeOfStruct = sizeof(Line);

  memset(&Module, 0, sizeof(Module));
  Module.SizeOfStruct = sizeof(Module);

  for (frameNum = 0; ; ++frameNum )
  {
    // get next stack frame (StackWalk64(), SymFunctionTableAccess64(), SymGetModuleBase64())
    // if this returns ERROR_INVALID_ADDRESS (487) or ERROR_NOACCESS (998), you can
    // assume that either you are done, or that the stack is so hosed that the next
    // deeper frame could not be found.
    // CONTEXT need not to be suplied if imageTyp is IMAGE_FILE_MACHINE_I386!
    if ( ! this->m_sw->pSW(imageType, this->m_hProcess, hThread, &s, &c, myReadProcMem, this->m_sw->pSFTA, this->m_sw->pSGMB, NULL) )
    {
      // INFO: "StackWalk64" does not set "GetLastError"...
      this->OnDbgHelpErr(_T("StackWalk64"), 0, s.AddrPC.Offset);
      break;
    }

    csEntry.offset = s.AddrPC.Offset;
    csEntry.name[0] = 0;
    csEntry.undName[0] = 0;
    csEntry.undFullName[0] = 0;
    csEntry.offsetFromSmybol = 0;
    csEntry.offsetFromLine = 0;
    csEntry.lineFileName[0] = 0;
    csEntry.lineNumber = 0;
    csEntry.loadedImageName[0] = 0;
    csEntry.moduleName[0] = 0;
    if (s.AddrPC.Offset == s.AddrReturn.Offset)
    {
      if ( (this->m_MaxRecursionCount > 0) && (curRecursionCount > m_MaxRecursionCount) )
      {
        this->OnDbgHelpErr(_T("StackWalk64-Endless-Callstack!"), 0, s.AddrPC.Offset);
        break;
      }
      curRecursionCount++;
    }
    else
      curRecursionCount = 0;
    if (s.AddrPC.Offset != 0)
    {
      // we seem to have a valid PC
      // show procedure info (SymGetSymFromAddr64())
      if (this->m_sw->pSGSFA(this->m_hProcess, s.AddrPC.Offset, &(csEntry.offsetFromSmybol), pSym) != FALSE)
      {
        _tcsncpy_s(csEntry.name, STACKWALK_MAX_NAMELEN, pSym->Name, _TRUNCATE);
        // UnDecorateSymbolName()
        this->m_sw->pUDSN( pSym->Name, csEntry.undName, STACKWALK_MAX_NAMELEN, UNDNAME_NAME_ONLY );
        this->m_sw->pUDSN( pSym->Name, csEntry.undFullName, STACKWALK_MAX_NAMELEN, UNDNAME_COMPLETE );
      }
      else
      {
        this->OnDbgHelpErr(_T("SymGetSymFromAddr64"), GetLastError(), s.AddrPC.Offset);
      }

      // show line number info, NT5.0-method (SymGetLineFromAddr64())
      if (this->m_sw->pSGLFA != NULL )
      { // yes, we have SymGetLineFromAddr64()
        if (this->m_sw->pSGLFA(this->m_hProcess, s.AddrPC.Offset, &(csEntry.offsetFromLine), &Line) != FALSE)
        {
          csEntry.lineNumber = Line.LineNumber;
          wcscpy_s(csEntry.lineFileName, Line.FileName);
        }
        else
        {
          this->OnDbgHelpErr(_T("SymGetLineFromAddr64"), GetLastError(), s.AddrPC.Offset);
        }
      } // yes, we have SymGetLineFromAddr64()

      // show module info (SymGetModuleInfo64())
      if (this->m_sw->GetModuleInfo(this->m_hProcess, s.AddrPC.Offset, &Module ) != FALSE)
      { // got module info OK
        switch ( Module.SymType )
        {
        case SymNone:
          csEntry.symTypeString = _T("-nosymbols-");
          break;
        case SymCoff:
          csEntry.symTypeString = _T("COFF");
          break;
        case SymCv:
          csEntry.symTypeString = _T("CV");
          break;
        case SymPdb:
          csEntry.symTypeString = _T("PDB");
          break;
        case SymExport:
          csEntry.symTypeString = _T("-exported-");
          break;
        case SymDeferred:
          csEntry.symTypeString = _T("-deferred-");
          break;
        case SymSym:
          csEntry.symTypeString = _T("SYM");
          break;
#if API_VERSION_NUMBER >= 9
        case SymDia:
          csEntry.symTypeString = _T("DIA");
          break;
#endif
        case 8: //SymVirtual:
          csEntry.symTypeString = _T("Virtual");
          break;
        default:
          //_snprintf( ty, sizeof ty, "symtype=%ld", (long) Module.SymType );
          csEntry.symTypeString = NULL;
          break;
        }

        wcscpy_s(csEntry.moduleName, Module.ModuleName);
        csEntry.baseOfImage = Module.BaseOfImage;
        wcscpy_s(csEntry.loadedImageName, Module.LoadedImageName);
      } // got module info OK
      else
      {
        this->OnDbgHelpErr(_T("SymGetModuleInfo64"), GetLastError(), s.AddrPC.Offset);
      }
    } // we seem to have a valid PC

    CallstackEntryType et = nextEntry;
    if (frameNum == 0)
      et = firstEntry;
    bLastEntryCalled = false;
    this->OnCallstackEntry(et, csEntry);
    
    if (s.AddrReturn.Offset == 0)
    {
      bLastEntryCalled = true;
      this->OnCallstackEntry(lastEntry, csEntry);
      SetLastError(ERROR_SUCCESS);
      break;
    }
  } // for ( frameNum )

  cleanup:
    if (pSym) free( pSym );

  if (bLastEntryCalled == false)
      this->OnCallstackEntry(lastEntry, csEntry);

  if (context == NULL)
    ResumeThread(hThread);

  return TRUE;
}

BOOL __stdcall CStackWalker::myReadProcMem(
    HANDLE      hProcess,
    DWORD64     qwBaseAddress,
    PVOID       lpBuffer,
    DWORD       nSize,
    LPDWORD     lpNumberOfBytesRead
    )
{
  if (s_readMemoryFunction == NULL)
  {
    SIZE_T st;
    BOOL bRet = ReadProcessMemory(hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, &st);
    *lpNumberOfBytesRead = (DWORD) st;
    //printf("ReadMemory: hProcess: %p, baseAddr: %p, buffer: %p, size: %d, read: %d, result: %d\n", hProcess, (LPVOID) qwBaseAddress, lpBuffer, nSize, (DWORD) st, (DWORD) bRet);
    return bRet;
  }
  else
  {
    return s_readMemoryFunction(hProcess, qwBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead, s_readMemoryFunction_UserData);
  }
}

void CStackWalker::OnLoadModule(LPCTSTR img, LPCTSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCTSTR symType, LPCTSTR pdbName, ULONGLONG fileVersion)
{
  TCHAR buffer[STACKWALK_MAX_NAMELEN];
  if (fileVersion == 0)
    _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN, _T("%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s'\n"), img, mod, (LPVOID) baseAddr, size, result, symType, pdbName);
  else
  {
    DWORD v4 = (DWORD) fileVersion & 0xFFFF;
    DWORD v3 = (DWORD) (fileVersion>>16) & 0xFFFF;
    DWORD v2 = (DWORD) (fileVersion>>32) & 0xFFFF;
    DWORD v1 = (DWORD) (fileVersion>>48) & 0xFFFF;
    _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN,_T( "%s:%s (%p), size: %d (result: %d), SymType: '%s', PDB: '%s', fileVersion: %d.%d.%d.%d\n"), img, mod, (LPVOID) baseAddr, size, result, symType, pdbName, v1, v2, v3, v4);
  }
  
  //OnOutput(buffer);
}

void CStackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
{
  TCHAR buffer[STACKWALK_MAX_NAMELEN];
  if ( (eType != lastEntry) && (entry.offset != 0) )
  {
    if (entry.name[0] == 0)
      wcscpy_s(entry.name, _T("(function-name not available)"));
    if (entry.undName[0] != 0)
      wcscpy_s(entry.name, entry.undName);
    if (entry.undFullName[0] != 0)
      wcscpy_s(entry.name, entry.undFullName);
    if (entry.lineFileName[0] == 0)
    {
      wcscpy_s(entry.lineFileName, _T("(filename not available)"));
      if (entry.moduleName[0] == 0)
        wcscpy_s(entry.moduleName, _T("(module-name not available)"));
      _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN, _T("%p (%s): %s: %s\n"), (LPVOID) entry.offset, entry.moduleName, entry.lineFileName, entry.name);
    }
    else
      _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN, _T("%s (%d): %s\n"), entry.lineFileName, entry.lineNumber, entry.name);
    OnOutput(buffer);
  }
}

void CStackWalker::OnDbgHelpErr(LPCTSTR szFuncName, DWORD gle, DWORD64 addr)
{
  TCHAR buffer[STACKWALK_MAX_NAMELEN];
  _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN, _T("ERROR: %s, GetLastError: %d (Address: %p)\n"), szFuncName, gle, (LPVOID) addr);
  // OnOutput(buffer);
}

void CStackWalker::OnSymInit(LPCTSTR szSearchPath, DWORD symOptions, LPCTSTR szUserName)
{
  TCHAR buffer[STACKWALK_MAX_NAMELEN];
  _snwprintf_s(buffer, STACKWALK_MAX_NAMELEN, _T("SymInit: Symbol-SearchPath: '%s', symOptions: %d, UserName: '%s'\n"), szSearchPath, symOptions, szUserName);
 // OnOutput(buffer);

}

void CStackWalker::OnOutput(LPCTSTR buffer)
{
  //OutputDebugStringA(buffer);
	if (mOutStream)
	{
		*mOutStream << buffer;
	}
	else
	{
		printf("%s", buffer);
	}
	
}

void CStackWalker::SetOutputStream(tofstream* outStream)
{
	mOutStream = outStream;
}

