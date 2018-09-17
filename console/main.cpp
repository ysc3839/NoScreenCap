#include <WinSDKVer.h>
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN64
#define TARGET_BIT 64
#define my_strtoul strtoull
#else
#define TARGET_BIT 32
#define my_strtoul strtoul
#endif

#define _STR(s) #s
#define STR(s) _STR(s)

#include "../core/core.hpp"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		fputs("Usage: NoScreenCap" STR(TARGET_BIT) ".com <window handle> [disable capture(0/1)]", stderr);
	}
	else
	{
		HWND hWnd = reinterpret_cast<HWND>(my_strtoul(argv[1], nullptr, 0));
		if (argc == 2)
		{
			DWORD affinity;
			if (GetWindowDisplayAffinity(hWnd, &affinity))
			{
				printf("%d", affinity);
				return 0;
			}
			else
			{
				fprintf(stderr, "GetWindowDisplayAffinity failed. (%u)", GetLastError());
			}
		}
		else
		{
			AdjustDebugPrivs();

			DWORD affinity = strtoul(argv[2], nullptr, 10);
			if (SetWindowDisplayAffinityForExternelProcess(hWnd, affinity))
				return 0;
		}
	}
	return 1;
}
