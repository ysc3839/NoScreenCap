#pragma once

#include "shellcode.hpp"

#ifdef _WIN64
#define CODE_SIZE CODE_X64_SIZE
#define BuildCode(f, a, b) BuildX64Code(f, a)
#else
#define CODE_SIZE CODE_X86_SIZE
#define BuildCode BuildX86Code
#endif

bool SetWindowDisplayAffinityForExternelProcess(HWND hWnd, DWORD affinity)
{
	bool retval = false;
	DWORD process_id;
	if (GetWindowThreadProcessId(hWnd, &process_id))
	{
		if (process_id == GetCurrentProcessId())
		{
			SetWindowDisplayAffinity(hWnd, affinity);
		}
		else
		{
			HANDLE hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, process_id);
			if (hProcess)
			{
				void *func_addr = reinterpret_cast<void*>(SetWindowDisplayAffinity);
				void *code_address = VirtualAllocEx(hProcess, nullptr, CODE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (code_address)
				{
					BuildCode(func_addr, affinity, code_address);
					if (WriteProcessMemory(hProcess, code_address, code, CODE_SIZE, nullptr))
					{
						HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(code_address), hWnd, 0, nullptr);
						if (hThread)
						{
							WaitForSingleObject(hThread, INFINITE);
							CloseHandle(hThread);
							retval = true;
						}
					}
					VirtualFreeEx(hProcess, code_address, sizeof(code), MEM_DECOMMIT);
				}
				CloseHandle(hProcess);
			}
		}
	}
	return retval;
}

bool AdjustDebugPrivs()
{
	bool retval = false;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		TOKEN_PRIVILEGES tp{};
		if (LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		{
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			retval = AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
		}
		CloseHandle(hToken);
	}
	return retval;
}
