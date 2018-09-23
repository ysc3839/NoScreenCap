#pragma once

#include "shellcode.hpp"

#ifndef _WIN64
#define CODE_SIZE CODE_X86_SIZE
#define BuildCode BuildX86Code
#endif // _WIN64

HANDLE WriteAndExecuteCode(HANDLE hProcess, void *code_address, void *code, size_t code_size, void *parameter = nullptr)
{
	if (WriteProcessMemory(hProcess, code_address, code, code_size, nullptr))
		return CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(code_address), parameter, 0, nullptr);
	return nullptr;
}

bool WriteAndExecuteCodeWait(HANDLE hProcess, void *code_address, void *code, size_t code_size, void *parameter = nullptr, DWORD timeout = INFINITE)
{
	HANDLE hThread = WriteAndExecuteCode(hProcess, code_address, code, code_size, parameter);
	if (hThread)
	{
		WaitForSingleObject(hThread, timeout);
		CloseHandle(hThread);
		return true;
	}
	return false;
}

bool WriteAndExecuteCodeWaitReturn(HANDLE hProcess, void *code_address, void *code, size_t code_size, DWORD *exit_code, void *parameter = nullptr, DWORD timeout = INFINITE)
{
	HANDLE hThread = WriteAndExecuteCode(hProcess, code_address, code, code_size, parameter);
	if (hThread)
	{
		WaitForSingleObject(hThread, timeout);
		GetExitCodeThread(hThread, exit_code);
		CloseHandle(hThread);
		return true;
	}
	return false;
}

#ifdef _WIN64
uint32_t GetFuncAddressX86(HANDLE hProcess)
{
	uint32_t retval = 0;
	void *code_address = VirtualAllocEx(hProcess, nullptr, CODE_GET_FUNC_ADDRESS_X86_SIZE_ALL, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (code_address)
	{
		BuildGetFuncCode(code_address);
		DWORD exit_code;
		if (WriteAndExecuteCodeWaitReturn(hProcess, code_address, code, CODE_GET_FUNC_ADDRESS_X86_SIZE_ALL, &exit_code))
			retval = exit_code;
		VirtualFreeEx(hProcess, code_address, CODE_GET_FUNC_ADDRESS_X86_SIZE_ALL, MEM_DECOMMIT);
	}
	return retval;
}
#endif // _WIN64

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
#ifdef _WIN64
				BOOL is_x86_process = false;
				if (!IsWow64Process(hProcess, &is_x86_process))
					is_x86_process = false;

				const size_t CODE_SIZE = is_x86_process ? CODE_X86_SIZE : CODE_X64_SIZE;
				void *func_addr = is_x86_process ? reinterpret_cast<void*>(static_cast<size_t>(GetFuncAddressX86(hProcess))) : reinterpret_cast<void*>(SetWindowDisplayAffinity);
#else
				void *func_addr = reinterpret_cast<void*>(SetWindowDisplayAffinity);
#endif // _WIN64
				void *code_address = VirtualAllocEx(hProcess, nullptr, CODE_SIZE, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
				if (code_address)
				{
#ifdef _WIN64
					if (!is_x86_process)
					{
						BuildX64Code(func_addr, affinity);
						retval = WriteAndExecuteCodeWait(hProcess, code_address, code, CODE_SIZE, hWnd);
					}
					else
#endif // _WIN64
					{
						BuildX86Code(func_addr, affinity, code_address);
						retval = WriteAndExecuteCodeWait(hProcess, code_address, code, CODE_SIZE, hWnd);
					}
					VirtualFreeEx(hProcess, code_address, CODE_SIZE, MEM_DECOMMIT);
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
