#pragma once

#define CODE_X64 \
	0xBA, 0x00, 0x00, 0x00, 0x00, /* mov edx, <affinity> */ \
	0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* mov rax, <func_addr> */ \
	0xFF, 0xE0 /* jmp rax */

constexpr uint8_t _CODE_X64[] = { CODE_X64 };
constexpr auto CODE_X64_SIZE = sizeof(_CODE_X64);

#define BuildX64Code(func_addr, affinity) \
	uint8_t code[] = { CODE_X64 }; \
	*reinterpret_cast<DWORD*>(&code[1]) = static_cast<DWORD>(affinity); \
	*reinterpret_cast<void**>(&code[7]) = static_cast<void*>(func_addr);

#define CODE_X86 \
	0x58, /* pop eax */ \
	0x59, /* pop ecx */ \
	0x6A, 0x00, /* push <affinity> */ \
	0x51, /* push ecx */ \
	0x50, /* push eax */ \
	0xE9, 0x00, 0x00, 0x00, 0x00 /* jmp <func_addr> */

constexpr uint8_t _CODE_X86[] = { CODE_X86 };
constexpr auto CODE_X86_SIZE = sizeof(_CODE_X86);

#define BuildX86Code(func_addr, affinity, base_address) \
	uint8_t code[] = { CODE_X86 }; \
	code[3] = static_cast<uint8_t>(affinity); \
	*reinterpret_cast<void**>(&code[7]) = reinterpret_cast<void*>(reinterpret_cast<size_t>(func_addr) - (reinterpret_cast<size_t>(base_address) + 6) - 5);
