#include "gui.h"

HINSTANCE g_hInst;

HWND g_hWnd;
HWND g_hWndLVWindows;
HIMAGELIST g_himlLV;

std::vector<windowInfo> windowsList;

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

inline bool GetWindowDisabledCapture(HWND hWnd)
{
	DWORD affinity;
	return (GetWindowDisplayAffinity(hWnd, &affinity) && affinity == WDA_MONITOR);
}

inline void ToggleDisabledCapture(windowInfo &info)
{
	SetWindowDisplayAffinityForExternelProcess(info.hWnd, info.disabledCapture ? WDA_NONE : WDA_MONITOR);
	info.disabledCapture = GetWindowDisabledCapture(info.hWnd);
}

void ReloadWindowsList()
{
	windowsList.clear();
	ImageList_RemoveAll(g_himlLV);

	static HICON hDefIcon = LoadIconW(nullptr, IDI_APPLICATION);
	static HICON hDefIconNew = static_cast<HICON>(LoadImageW(LoadLibraryExW(L"imageres.dll", nullptr,
		LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE | LOAD_LIBRARY_SEARCH_SYSTEM32),
		MAKEINTRESOURCEW(15), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0));

	static int iDefIcon;
	iDefIcon = ImageList_AddIcon(g_himlLV, hDefIconNew);

	EnumWindows([](HWND hWnd, LPARAM) -> BOOL {
		if (!IsWindowVisible(hWnd)) return TRUE;

		windowInfo info;
		info.hWnd = hWnd;

		HICON hIcon = nullptr;
		auto result = SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&hIcon));
		if (!result || !hIcon || hIcon == hDefIcon)
		{
			hIcon = nullptr;
			result = SendMessageTimeoutW(hWnd, WM_GETICON, ICON_SMALL2, 0, SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&hIcon));
			if (!result || !hIcon || hIcon == hDefIcon)
			{
				hIcon = reinterpret_cast<HICON>(GetClassLongPtrW(hWnd, GCLP_HICON));
				if (!hIcon || hIcon == hDefIcon)
				{
					result = SendMessageTimeoutW(hWnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG, 1000, reinterpret_cast<PDWORD_PTR>(&hIcon));
					if (!result || !hIcon || hIcon == hDefIcon)
					{
						hIcon = reinterpret_cast<HICON>(GetClassLongPtrW(hWnd, GCLP_HICON));
					}
				}
			}
		}

		if (hIcon && hIcon != hDefIcon)
			info.iIcon = ImageList_AddIcon(g_himlLV, hIcon);
		else
			info.iIcon = iDefIcon;

		int textLen = GetWindowTextLengthW(hWnd);
		if (textLen != 0)
		{
			++textLen;
			info.caption.resize(textLen);
			if (GetWindowTextW(hWnd, info.caption.data(), textLen) == 0)
				info.caption.clear();
		}

		info.className.resize(MAX_PATH);
		if (GetClassNameW(hWnd, info.className.data(), MAX_PATH) == 0)
			info.className.clear();

		info.disabledCapture = GetWindowDisabledCapture(hWnd);

		windowsList.emplace_back(info);

		return TRUE;
	}, 0);
}

void LVOnDispInfo(LPNMLVDISPINFOW di)
{
	size_t i = di->item.iItem;
	auto &info = windowsList[i];
	if (di->item.mask & LVIF_TEXT)
	{
		switch (di->item.iSubItem)
		{
		case 1:
			info.caption.copy(di->item.pszText, di->item.cchTextMax);
			break;
		case 2:
			info.className.copy(di->item.pszText, di->item.cchTextMax);
			break;
		case 3:
			swprintf_s(di->item.pszText, di->item.cchTextMax, L"%zX", reinterpret_cast<size_t>(info.hWnd));
			break;
		}
	}
	if (di->item.mask & LVIF_STATE && di->item.iSubItem == 0)
	{
		di->item.stateMask = LVIS_STATEIMAGEMASK;
		di->item.state = INDEXTOSTATEIMAGEMASK(info.disabledCapture ? 1 : 2);
	}
	if (di->item.mask & LVIF_IMAGE && di->item.iSubItem == 1)
	{
		di->item.iImage = info.iIcon;
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HFONT _hFont = nullptr;
	switch (message)
	{
	case WM_CREATE:
	{
		if (g_darkModeSupported)
		{
			_AllowDarkModeForWindow(hWnd, true);
			RefreshTitleBarThemeColor(hWnd);
		}

		HFONT hFont;
		NONCLIENTMETRICSW ncm{ sizeof(ncm) };
		if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0) &&
			(hFont = CreateFontIndirectW(&ncm.lfMessageFont)))
			_hFont = hFont;
		else
			hFont = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

		g_hWndLVWindows = CreateWindowW(WC_LISTVIEWW, nullptr, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDATA, 0, 0, 0, 0, hWnd, nullptr, g_hInst, nullptr);
		if (g_hWndLVWindows)
		{
			InitListView(g_hWndLVWindows);

			LVCOLUMNW lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = 0;
			ListView_InsertColumn(g_hWndLVWindows, 0, &lvc);

			lvc.mask = LVCF_FMT | LVCF_WIDTH;
			lvc.cx = Scale(20);
			lvc.fmt = LVCFMT_FIXED_WIDTH;
			ListView_InsertColumn(g_hWndLVWindows, 1, &lvc);

			lvc.mask = LVCF_WIDTH | LVCF_TEXT;
			lvc.cx = Scale(240);
			lvc.pszText = const_cast<LPWSTR>(L"Caption");
			ListView_InsertColumn(g_hWndLVWindows, 2, &lvc);

			lvc.cx = Scale(240);
			lvc.pszText = const_cast<LPWSTR>(L"Class");
			ListView_InsertColumn(g_hWndLVWindows, 3, &lvc);

			lvc.cx = Scale(80);
			lvc.pszText = const_cast<LPWSTR>(L"Handle");
			ListView_InsertColumn(g_hWndLVWindows, 4, &lvc);

			ListView_DeleteColumn(g_hWndLVWindows, 0);

			g_himlLV = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32, 0, 0);
			ListView_SetImageList(g_hWndLVWindows, g_himlLV, LVSIL_SMALL);
		}

		ReloadWindowsList();
		ListView_SetItemCount(g_hWndLVWindows, windowsList.size());
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_RELOAD:
			ReloadWindowsList();
			ListView_SetItemCount(g_hWndLVWindows, windowsList.size());
			break;
		case IDM_ABOUT:
			DialogBoxW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_NOTIFY:
	{
		if (reinterpret_cast<LPNMHDR>(lParam)->hwndFrom == g_hWndLVWindows)
		{
			switch (reinterpret_cast<LPNMHDR>(lParam)->code)
			{
			case LVN_GETDISPINFO:
			{
				auto di = reinterpret_cast<LPNMLVDISPINFOW>(lParam);
				LVOnDispInfo(di);
			}
			break;
			case LVN_ODFINDITEM:
			{
				auto fi = reinterpret_cast<LPNMLVFINDITEMW>(lParam);
				auto flags = fi->lvfi.flags;

				if (!(flags & (LVFI_STRING | LVFI_PARTIAL | LVFI_SUBSTRING)))
					return -1;

				bool wrap = (flags & LVFI_WRAP);
				auto start = windowsList.cbegin() + fi->iStart, end = windowsList.cend();

				while (1)
				{
					for (auto &it = start; it != end; ++it)
					{
						if (flags & (LVFI_PARTIAL | LVFI_SUBSTRING))
						{
							size_t len = wcslen(fi->lvfi.psz);
							if (len > it->caption.size() || _wcsnicmp(fi->lvfi.psz, it->caption.c_str(), len) != 0)
								continue;
						}
						else
						{
							if (_wcsnicmp(it->caption.c_str(), fi->lvfi.psz, it->caption.size()) != 0)
								continue;
						}

						return it - windowsList.cbegin();
					}

					if (wrap)
					{
						wrap = false;
						start = windowsList.cbegin();
						end = windowsList.cbegin() + fi->iStart;
					}
					else
						break;
				}
				return -1;
			}
			break;
			case NM_CLICK:
			{
				auto item = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
				if (item->iItem != -1)
				{
					LVHITTESTINFO hitInfo;
					hitInfo.pt = item->ptAction;
					int i = ListView_HitTest(g_hWndLVWindows, &hitInfo);
					if (i != -1)
					{
						if ((hitInfo.flags & LVHT_ONITEM) != LVHT_ONITEM && (hitInfo.flags & LVHT_ONITEMSTATEICON))
						{
							ToggleDisabledCapture(windowsList[i]);
							ListView_RedrawItems(g_hWndLVWindows, i, i);
						}
					}
				}
			}
			break;
			case LVN_ITEMACTIVATE:
			{
				auto item = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
				int i = item->iItem;
				ToggleDisabledCapture(windowsList[i]);
				ListView_RedrawItems(g_hWndLVWindows, i, i);
			}
			break;
			}
		}
	}
	break;
	case WM_DESTROY:
		if (_hFont)
			DeleteFont(_hFont);
		PostQuitMessage(0);
		break;
	case WM_SIZE:
	{
		int clientWidth = GET_X_LPARAM(lParam), clientHeight = GET_Y_LPARAM(lParam);
		HDWP hDWP = BeginDeferWindowPos(1);
		if (hDWP != nullptr)
		{
			DeferWindowPos(hDWP, g_hWndLVWindows, 0, 0, 0, clientWidth, clientHeight, SWP_NOZORDER);
			EndDeferWindowPos(hDWP);
		}
	}
	break;
	case WM_SETTINGCHANGE:
	{
		if (IsColorSchemeChangeMessage(lParam))
		{
			RefreshTitleBarThemeColor(hWnd);
			SendMessageW(g_hWndLVWindows, WM_THEMECHANGED, 0, 0);
		}
	}
	break;
	default:
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitDPIScale();
	InitDarkMode();

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_GUI));
	wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GUI);
	wcex.lpszClassName = L"NoScreenCap";
	//wcex.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_SMALL));

	RegisterClassExW(&wcex);

	g_hInst = hInstance;

	g_hWnd = CreateWindowExW(0, L"NoScreenCap", L"NoScreenCap", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!g_hWnd)
		return FALSE;

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);

	MSG msg;
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	return (int)msg.wParam;
}
