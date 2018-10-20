#pragma once

using fnRtlGetNtVersionNumbers = void (WINAPI *)(LPDWORD major, LPDWORD minor, LPDWORD build);
using fnShouldAppsUseDarkMode = bool (WINAPI *)();
using fnAllowDarkModeForWindow = bool (WINAPI *)(HWND hWnd, bool allow);
using fnAllowDarkModeForApp = bool (WINAPI *)(bool allow);
using fnFlushMenuThemes = void (WINAPI *)();
using fnRefreshImmersiveColorPolicyState = void (WINAPI *)();

fnShouldAppsUseDarkMode _ShouldAppsUseDarkMode = nullptr;
fnAllowDarkModeForWindow _AllowDarkModeForWindow = nullptr;
fnAllowDarkModeForApp _AllowDarkModeForApp = nullptr;
fnFlushMenuThemes _FlushMenuThemes = nullptr;
fnRefreshImmersiveColorPolicyState _RefreshImmersiveColorPolicyState = nullptr;

bool g_darkModeSupported = false;

void InitDarkMode()
{
	fnRtlGetNtVersionNumbers RtlGetNtVersionNumbers = reinterpret_cast<fnRtlGetNtVersionNumbers>(GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetNtVersionNumbers"));
	if (RtlGetNtVersionNumbers)
	{
		DWORD major, minor, build;
		RtlGetNtVersionNumbers(&major, &minor, &build);
		if (major == 10 && minor == 0 && build == (17763 | 0xF0000000)) // Windows 10 1809 10.0.17763
		{
			HMODULE hUxtheme = LoadLibraryW(L"uxtheme.dll");
			if (hUxtheme)
			{
				_RefreshImmersiveColorPolicyState = reinterpret_cast<fnRefreshImmersiveColorPolicyState>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(104)));
				_ShouldAppsUseDarkMode = reinterpret_cast<fnShouldAppsUseDarkMode>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(132)));
				_AllowDarkModeForWindow = reinterpret_cast<fnAllowDarkModeForWindow>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(133)));
				_AllowDarkModeForApp = reinterpret_cast<fnAllowDarkModeForApp>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(135)));
				_FlushMenuThemes = reinterpret_cast<fnFlushMenuThemes>(GetProcAddress(hUxtheme, MAKEINTRESOURCEA(136)));

				if (_RefreshImmersiveColorPolicyState &&
					_ShouldAppsUseDarkMode &&
					_AllowDarkModeForWindow &&
					_AllowDarkModeForApp &&
					_FlushMenuThemes)
				{
					_AllowDarkModeForApp(true);
					_FlushMenuThemes();

					g_darkModeSupported = true;
				}
			}
		}
	}
}

inline bool AllowDarkModeForWindow(HWND hWnd, bool allow)
{
	if (g_darkModeSupported)
		return _AllowDarkModeForWindow(hWnd, allow);
	return false;
}
