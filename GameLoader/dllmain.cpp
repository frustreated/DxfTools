#include "stdafx.h"
#include "LoadPE.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
#pragma comment(linker, "/EXPORT:DriverProc=_AheadLib_DriverProc,@1")
#pragma comment(linker, "/EXPORT:widMessage=_AheadLib_widMessage,@2")
#pragma comment(linker, "/EXPORT:wodMessage=_AheadLib_wodMessage,@3")
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UNIQUE_MAC_MODULE_ADDR L"092FC6F5-074E-43EE-AB2F-B5E73F7D7EC9-%d"
#define UNIQUE_MAC_NO_MODULE_ADDR L"092FC6F5-074E-43EE-AB2F-B5E73F7D7EC8-%d"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 原函数地址指针
PVOID pfnDriverProc;
PVOID pfnwidMessage;
PVOID pfnwodMessage;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InstallMemLoadDll(HMODULE hModule);
bool IsMutexExist(std::wstring name);


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 宏定义
#define EXTERNC extern "C"
#define NAKED __declspec(naked)
#define EXPORT __declspec(dllexport)

#define ALCPP EXPORT NAKED
#define ALSTD EXTERNC EXPORT NAKED void __stdcall
#define ALCFAST EXTERNC EXPORT NAKED void __fastcall
#define ALCDECL EXTERNC NAKED void __cdecl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AheadLib 命名空间
namespace AheadLib
{
	HMODULE m_hModule = NULL;	// 原始模块句柄
	DWORD m_dwReturn[3] = { 0 };	// 原始函数返回地址
	inline BOOL WINAPI Load();

	// 获取原始函数地址
	FARPROC WINAPI GetAddress(PCSTR pszProcName)
	{
		FARPROC fpAddress;
		CHAR szProcName[16];
		TCHAR tzTemp[MAX_PATH];

		if (m_hModule == NULL)
		{
			if (Load() == FALSE)
			{
				ExitProcess(-1);
			}
		}

		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == NULL)
		{
			if (HIWORD(pszProcName) == 0)
			{
				wsprintfA(szProcName, "%d", pszProcName);
				pszProcName = szProcName;
			}

			wsprintf(tzTemp, TEXT("无法找到函数 %hs，程序无法正常运行。"), pszProcName);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
			ExitProcess(-2);
		}

		return fpAddress;
	}

	// 初始化原始函数地址指针
	inline VOID WINAPI InitializeAddresses()
	{
		pfnDriverProc = GetAddress("DriverProc");
		pfnwidMessage = GetAddress("widMessage");
		pfnwodMessage = GetAddress("wodMessage");
	}

	// 加载原始模块
	inline BOOL WINAPI Load()
	{
		TCHAR tzPath[MAX_PATH];
		TCHAR tzTemp[MAX_PATH * 2];

		GetSystemDirectory(tzPath, MAX_PATH);

		LoadLibraryW(L"samcli.dll");
		lstrcat(tzPath, TEXT("\\msacm32.drv"));
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == NULL)
		{
			wsprintf(tzTemp, TEXT("无法加载 %s，程序无法正常运行。"), tzPath);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		}
		else
		{
			InitializeAddresses();
		}

		return (m_hModule != NULL);
	}

	// 释放原始模块
	inline VOID WINAPI Free()
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
	}
}
using namespace AheadLib;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 入口函数
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);
		//InstallMemLoadDll(hModule);
		Load();
		return FALSE;
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		Free();
	}

	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_DriverProc(void)
{
	__asm JMP pfnDriverProc;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_widMessage(void)
{
	__asm JMP pfnwidMessage;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 导出函数
ALCDECL AheadLib_wodMessage(void)
{
	__asm JMP pfnwodMessage;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void InstallMemLoadDll(HMODULE hModule)
{
	//char szMutexName[MAX_PATH] = { 0 };
	//wsprintf(szMutexName, UNIQUE_MAC_MODULE_ADDR, GetCurrentProcessId());
	std::wstringstream ss;
	ss << UNIQUE_MAC_MODULE_ADDR << GetCurrentProcess();

	//if (!IsMutexExist(ss.str())) {
		//正常逻辑
		//auto mutex =  CreateMutex(NULL, TRUE, ss.str().c_str());
		char szDllPath[] = "c:\\game.dll";
		LaunchDll(szDllPath, DLL_PROCESS_ATTACH);
	//}
}

bool IsMutexExist(std::wstring name)
{
	if (name.empty()) {
		return false;
	}

	auto hMutex = CreateMutex(NULL, TRUE, name.c_str());
	if (!hMutex) {
		return false;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return true;
	}

	ReleaseMutex(hMutex);
	return false;
}
