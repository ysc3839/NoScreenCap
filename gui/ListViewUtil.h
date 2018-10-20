#pragma once

void InitListViewHeader(HWND hHeader)
{
	AllowDarkModeForWindow(hHeader, true);
	SetWindowTheme(hHeader, L"ItemsView", nullptr); // DarkMode
}

void InitListView(HWND hListView)
{
	AllowDarkModeForWindow(hListView, true);
	SetWindowTheme(hListView, L"ItemsView", nullptr); // DarkMode

	HWND hHeader = ListView_GetHeader(hListView);
	InitListViewHeader(hHeader);

	ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_HEADERDRAGDROP | LVS_EX_CHECKBOXES | LVS_EX_SUBITEMIMAGES);

	// Hide focus dots
	SendMessage(hListView, WM_CHANGEUISTATE, MAKELONG(UIS_SET, UISF_HIDEFOCUS), 0);
}
