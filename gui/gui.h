#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <CommCtrl.h>
#include <Uxtheme.h>
#include <WindowsX.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Uxtheme.lib")

#include "resource.h"

#include "HiDPI.h"
#include "DarkMode.h"
#include "ListViewUtil.h"

#include "../core/core.hpp"

struct windowInfo
{
	HWND hWnd;
	int iIcon;
	std::wstring caption;
	std::wstring className;
	bool disabledCapture; // false - can be captured
};
