#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include "D:\C files\myWin32func.h"
#include "D:\C files\uiaccess.h"
#include <windowsx.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include <unordered_map>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#include "resource.h"
#pragma comment(linker,	"\"/manifestdependency:type='win32' \
							name='Microsoft.Windows.Common-Controls' \
							version='6.0.0.0' \
							processorArchitecture='*' \
							publicKeyToken='6595b64144ccf1df' \
							language='*'\"")
using namespace std;

//自定义消息
#define WM_TARGET WM_USER+1
#define WM_POS WM_USER+2
#define WM_SIZECHANGE WM_USER+3
#define WM_TRAY WM_USER+4
#define WM_FRAME WM_USER+5

//全局宏定义
constexpr int HANDLESIZE = 21;

//窗口风格类
constexpr struct { DWORD flag; LPCSTR name; } StyleFlags[] = {
	{ WS_BORDER,		"WS_BORDER" },
	{ WS_CAPTION,		"WS_CAPTION" },
	{ WS_CHILD,			"WS_CHILD" },
	{ WS_CHILDWINDOW,	"WS_CHILDWINDOW" },
	{ WS_CLIPCHILDREN,	"WS_CLIPCHILDREN" },
	{ WS_CLIPSIBLINGS,	"WS_CLIPSIBLINGS" },
	{ WS_DISABLED,		"WS_DISABLED" },
	{ WS_DLGFRAME,		"WS_DLGFRAME" },
	{ WS_GROUP,			"WS_GROUP" },
	{ WS_HSCROLL,		"WS_HSCROLL" },
	{ WS_ICONIC,		"WS_ICONIC" },
	{ WS_MAXIMIZE,		"WS_MAXIMIZE" },
	{ WS_MAXIMIZEBOX,	"WS_MAXIMIZEBOX" },
	{ WS_MINIMIZE,		"WS_MINIMIZE" },
	{ WS_MINIMIZEBOX,	"WS_MINIMIZEBOX" },
	{ WS_OVERLAPPED,	"WS_OVERLAPPED" },//0
	{ WS_OVERLAPPEDWINDOW,	"WS_OVERLAPPEDWINDOW" },
	{ WS_POPUP,			"WS_POPUP" },
	{ WS_POPUPWINDOW,	"WS_POPUPWINDOW" },
	{ WS_SIZEBOX,		"WS_SIZEBOX" },
	{ WS_SYSMENU,		"WS_SYSMENU" },
	{ WS_TABSTOP,		"WS_TABSTOP" },
	{ WS_THICKFRAME,	"WS_THICKFRAME" },
	{ WS_TILED,			"WS_TILED" },//0
	{ WS_TILEDWINDOW,	"WS_TILEDWINDOW" },
	{ WS_VISIBLE,		"WS_VISIBLE" },
	{ WS_VSCROLL,		"WS_VSCROLL" },
};
constexpr struct { DWORD flag; LPCSTR name; } ExStyleFlags[] = {
{ WS_EX_ACCEPTFILES,	"WS_EX_ACCEPTFILES" },
{ WS_EX_APPWINDOW,		"WS_EX_APPWINDOW" },
{ WS_EX_CLIENTEDGE,		"WS_EX_CLIENTEDGE" },
{ WS_EX_COMPOSITED,		"WS_EX_COMPOSITED" },
{ WS_EX_CONTEXTHELP,	"WS_EX_CONTEXTHELP" },
{ WS_EX_CONTROLPARENT,	"WS_EX_CONTROLPARENT" },
{ WS_EX_DLGMODALFRAME,	"WS_EX_DLGMODALFRAME" },
{ WS_EX_LAYERED,		"WS_EX_LAYERED" },
{ WS_EX_LAYOUTRTL,		"WS_EX_LAYOUTRTL" },
{ WS_EX_LEFT,			"WS_EX_LEFT" },//0
{ WS_EX_LEFTSCROLLBAR,	"WS_EX_LEFTSCROLLBAR" },
{ WS_EX_LTRREADING,		"WS_EX_LTRREADING" },//0
{ WS_EX_MDICHILD,		"WS_EX_MDICHILD" },
{ WS_EX_NOACTIVATE,		"WS_EX_NOACTIVATE" },
{ WS_EX_NOINHERITLAYOUT,"WS_EX_NOINHERITLAYOUT" },
{ WS_EX_NOPARENTNOTIFY,		"WS_EX_NOPARENTNOTIFY" },
#if _WIN32_WINNT > _WIN32_WINNT_WINXP
{ WS_EX_NOREDIRECTIONBITMAP,"WS_EX_NOREDIRECTIONBITMAP" },
#endif
{ WS_EX_OVERLAPPEDWINDOW,	"WS_EX_OVERLAPPEDWINDOW" },
{ WS_EX_PALETTEWINDOW,	"WS_EX_PALETTEWINDOW" },
{ WS_EX_RIGHT,			"WS_EX_RIGHT" },
{ WS_EX_RIGHTSCROLLBAR,	"WS_EX_RIGHTSCROLLBAR" },//0
{ WS_EX_RTLREADING,		"WS_EX_RTLREADING" },
{ WS_EX_STATICEDGE,		"WS_EX_STATICEDGE" },
{ WS_EX_TOOLWINDOW,		"WS_EX_TOOLWINDOW" },
{ WS_EX_TOPMOST,		"WS_EX_TOPMOST" },
{ WS_EX_TRANSPARENT,	"WS_EX_TRANSPARENT" },
{ WS_EX_WINDOWEDGE,		"WS_EX_WINDOWEDGE" },
};

//全局变量区
extern HWND g_MainHwnd;
extern HWND g_StaticClassname;
extern HWND g_EditTitlename;
extern HWND g_StaticExename;
extern HWND g_StaticStyle;
extern HWND g_StaticExStyle;
extern HWND g_EditTransparent;
extern HWND g_EditPosX;
extern HWND g_EditPosY;
extern HWND g_EditSizeX;
extern HWND g_EditSizeY;
extern HWND g_StaticInstance;

//函数声明
//wstring GetProcessNameFromWindow2(HWND hwnd);
wstring GetProcessNameFromWindow(HWND hwnd);
BOOL IsAdmin();
void ShakeWindow(HWND hWnd);
bool SetWindowTransparent(HWND &target, INT32 &trans);
void FillInformation(HWND target);
void ResetInformation();
LONG_PTR SetWindowThrough(HWND hWnd, BOOL enabled, LONG_PTR OldexStyle);