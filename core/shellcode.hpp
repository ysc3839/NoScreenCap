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

// Source code: shellcode_x86_get_func_address.c
#define CODE_GET_FUNC_ADDRESS_X86 \
	0x64, 0xA1, 0x30, 0x00, 0x00, 0x00,	/* mov eax,dword ptr fs:[00000030h] */ \
	0x53,								/* push ebx */ \
	0x56,								/* push esi */ \
	0x57,								/* push edi */ \
	0x8B, 0x40, 0x0C,					/* mov eax,dword ptr [eax+0Ch] */ \
	0x8B, 0x40, 0x14,					/* mov eax,dword ptr [eax+14h] */ \
	0x8B, 0x00,							/* mov eax,dword ptr [eax] */ \
	0x8B, 0x00,							/* mov eax,dword ptr [eax] */ \
	0x8B, 0x78, 0x10,					/* mov edi,dword ptr [eax+10h] */ \
	0x8B, 0x47, 0x3C,					/* mov eax,dword ptr [edi+3Ch] */ \
	0x8B, 0x44, 0x38, 0x78,				/* mov eax,dword ptr [eax+edi+78h] */ \
	0x8B, 0x4C, 0x38, 0x20,				/* mov ecx,dword ptr [eax+edi+20h] */ \
	0x8B, 0x74, 0x38, 0x24,				/* mov esi,dword ptr [eax+edi+24h] */ \
	0x03, 0xCF,							/* add ecx,edi */ \
	0x8B, 0x5C, 0x38, 0x1C,				/* mov ebx,dword ptr [eax+edi+1Ch] */ \
	0x03, 0xF7,							/* add esi,edi */ \
	0x03, 0xDF,							/* add ebx,edi */ \
	0x33, 0xD2,							/* xor edx,edx */ \
	/* label1: */ \
	0x8B, 0x01,							/* mov eax,dword ptr [ecx] */ \
	0x81, 0x3C, 0x38, 0x47, 0x65, 0x74, 0x50, /* cmp dword ptr [eax+edi],50746547h */ \
	0x75, 0x14,							/* jne <label2> */ \
	0x81, 0x7C, 0x38, 0x04, 0x72, 0x6F, 0x63, 0x41, /* cmp dword ptr [eax+edi+4],41636F72h */ \
	0x75, 0x0A,							/* jne <label2> */ \
	0x81, 0x7C, 0x38, 0x08, 0x64, 0x64, 0x72, 0x65, /* cmp dword ptr [eax+edi+8],65726464h */ \
	0x74, 0x06,							/* je <label3> */ \
	/* label2: */ \
	0x83, 0xC1, 0x04,					/* add ecx,4 */ \
	0x42,								/* inc edx */ \
	0xEB, 0xDB,							/* jmp <label1> */ \
	/* label3: */ \
	0x0F, 0xB7, 0x04, 0x56,				/* movzx eax,word ptr [esi+edx*2] */ \
	0x68, 0x00, 0x00, 0x00, 0x00,		/* push offset string "GetModuleHandleA" */ \
	0x57,								/* push edi */ \
	0x8B, 0x34, 0x83,					/* mov esi,dword ptr [ebx+eax*4] */ \
	0x03, 0xF7,							/* add esi,edi */ \
	0xFF, 0xD6,							/* call esi */ \
	0x68, 0x00, 0x00, 0x00, 0x00,		/* push offset string "SetWindowDisplayAffinity" */ \
	0x68, 0x00, 0x00, 0x00, 0x00,		/* push offset string "user32" */ \
	0xFF, 0xD0,							/* call eax */ \
	0x50,								/* push eax */ \
	0xFF, 0xD6,							/* call esi */ \
	0x5F,								/* pop edi */ \
	0x5E,								/* pop esi */ \
	0x5B,								/* pop ebx */ \
	0xC2, 0x04, 0x00					/* ret 4 */ \

#define CODE_GET_FUNC_ADDRESS_X86_DATA_GMH \
	'G','e','t','M','o','d','u','l','e','H','a','n','d','l','e','A',0
#define CODE_GET_FUNC_ADDRESS_X86_DATA_USER32 \
	'u','s','e','r','3','2',0
#define CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF \
	'S','e','t','W','i','n','d','o','w','D','i','s','p','l','a','y','A','f','f','i','n','i','t','y',0

constexpr uint8_t _CODE_GET_FUNC_ADDRESS_X86[] = { CODE_GET_FUNC_ADDRESS_X86 };
constexpr auto CODE_GET_FUNC_ADDRESS_X86_SIZE = sizeof(_CODE_GET_FUNC_ADDRESS_X86);

constexpr uint8_t _CODE_GET_FUNC_ADDRESS_X86_DATA_GMH[] = { CODE_GET_FUNC_ADDRESS_X86_DATA_GMH };
constexpr auto CODE_GET_FUNC_ADDRESS_X86_DATA_GMH_SIZE = sizeof(_CODE_GET_FUNC_ADDRESS_X86_DATA_GMH);

constexpr uint8_t _CODE_GET_FUNC_ADDRESS_X86_DATA_USER32[] = { CODE_GET_FUNC_ADDRESS_X86_DATA_USER32 };
constexpr auto CODE_GET_FUNC_ADDRESS_X86_DATA_USER32_SIZE = sizeof(_CODE_GET_FUNC_ADDRESS_X86_DATA_USER32);

constexpr uint8_t _CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF[] = { CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF };
constexpr auto CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF_SIZE = sizeof(_CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF);

constexpr auto CODE_GET_FUNC_ADDRESS_X86_SIZE_ALL = CODE_GET_FUNC_ADDRESS_X86_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_GMH_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_USER32_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF_SIZE;

#define BuildGetFuncCode(base_address) \
	uint8_t code[] = { CODE_GET_FUNC_ADDRESS_X86, CODE_GET_FUNC_ADDRESS_X86_DATA_GMH, CODE_GET_FUNC_ADDRESS_X86_DATA_USER32, CODE_GET_FUNC_ADDRESS_X86_DATA_SWDF }; \
	*reinterpret_cast<uint32_t*>(&code[91]) = static_cast<uint32_t>(reinterpret_cast<size_t>(base_address) + CODE_GET_FUNC_ADDRESS_X86_SIZE); \
	*reinterpret_cast<uint32_t*>(&code[104]) = static_cast<uint32_t>(reinterpret_cast<size_t>(base_address) + CODE_GET_FUNC_ADDRESS_X86_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_GMH_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_USER32_SIZE); \
	*reinterpret_cast<uint32_t*>(&code[109]) = static_cast<uint32_t>(reinterpret_cast<size_t>(base_address) + CODE_GET_FUNC_ADDRESS_X86_SIZE + CODE_GET_FUNC_ADDRESS_X86_DATA_GMH_SIZE);
