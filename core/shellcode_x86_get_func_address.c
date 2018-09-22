#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h>

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _LDR_DATA_TABLE_ENTRY {
	// PVOID Reserved1[2];
	LIST_ENTRY InMemoryOrderLinks;
	PVOID Reserved2[2];
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef struct _PEB_LDR_DATA {
	BYTE       Reserved1[8];
	PVOID      Reserved2[3];
	LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _PEB {
	BYTE                          Reserved1[2];
	BYTE                          BeingDebugged;
	BYTE                          Reserved2[1];
	PVOID                         Reserved3[2];
	PPEB_LDR_DATA                 Ldr;
} PEB, *PPEB;

int main()
{
	PPEB peb = (PPEB)__readfsdword(0x30);
	PLDR_DATA_TABLE_ENTRY entry = (PLDR_DATA_TABLE_ENTRY)peb->Ldr->InMemoryOrderModuleList.Flink->Flink->Flink; // The third is kernel32.
	uint32_t dll_base = (uint32_t)entry->DllBase;

	PIMAGE_NT_HEADERS32 nt_hdr = (PIMAGE_NT_HEADERS32)(dll_base + ((PIMAGE_DOS_HEADER)dll_base)->e_lfanew);
	PIMAGE_DATA_DIRECTORY data_dir = nt_hdr->OptionalHeader.DataDirectory;
	PIMAGE_EXPORT_DIRECTORY exp_dir = (PIMAGE_EXPORT_DIRECTORY)(dll_base + data_dir->VirtualAddress);

	uint32_t *func_names = (uint32_t *)(dll_base + exp_dir->AddressOfNames);
	uint16_t *name_ords = (uint16_t *)(dll_base + exp_dir->AddressOfNameOrdinals);
	uint32_t *funcs = (uint32_t *)(dll_base + exp_dir->AddressOfFunctions);

	size_t i = 0;
	while(1)
	{
		uint32_t *func_name = (uint32_t *)(dll_base + *func_names);
		if (func_name[0] == 0x50746547 && // GetP
			func_name[1] == 0x41636f72 && // rocA
			func_name[2] == 0x65726464)   // ddre
			break;
		++func_names;
		++i;
	}

	typedef FARPROC (__stdcall *fnGetProcAddress)(HMODULE hModule, LPCSTR lpProcName);
	fnGetProcAddress get_proc_address = (fnGetProcAddress)(dll_base + funcs[name_ords[i]]);

	typedef HMODULE (__stdcall *fnGetModuleHandleA)(LPCSTR lpModuleName);
	fnGetModuleHandleA get_module_handle = (fnGetModuleHandleA)get_proc_address((HMODULE)dll_base, "GetModuleHandleA");

	return get_proc_address(get_module_handle("user32"), "SetWindowDisplayAffinity");
}
