#pragma once

uint32_t g_dpiScale;

// https://stackoverflow.com/a/25065519
int muldiv(int number, int numerator, int denominator)
{
	return (int)(((long)number * numerator + (denominator >> 1)) / denominator);
}

/*void SetDPIAware()
{
	HMODULE hUser = GetModuleHandle(L"user32.dll");
	if (hUser)
	{
		BOOL (WINAPI *pSetProcessDPIAware)();
		pSetProcessDPIAware = GetProcAddress(hUser, "SetProcessDPIAware");
		if (pSetProcessDPIAware)
			pSetProcessDPIAware();
	}
}*/

void InitDPIScale()
{
	HDC hdc = GetDC(0);
	if (hdc)
	{
		g_dpiScale = GetDeviceCaps(hdc, LOGPIXELSX);
		if (g_dpiScale == 96)
			g_dpiScale = 0;
		else
			g_dpiScale = muldiv(g_dpiScale, 100, 96);
		ReleaseDC(0, hdc);
	}
}

int Scale(int i)
{
	if (g_dpiScale == 0)
		return i;
	else
		return muldiv(i, g_dpiScale, 100);
}
