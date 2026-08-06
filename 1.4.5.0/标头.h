#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <deque>
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
#define WM_Tray WM_USER+1
#define WM_Frame WM_USER+2
#define WM_Update WM_USER+3
#define WM_Mouse WM_USER+4
extern UINT WM_KeepTray;

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

//配置的位掩码
#define WH_TRANSPARENT	0b1
#define WH_POS			0b10
#define WH_SIZE			0b100
#define WH_ENABLE		0b1000
#define WH_TOPMOST		0b10000
#define WH_THROUGH		0b100000
#define WH_TITLE		0b1000000
#define WH_STYLE		0b10000000
#define WH_EXSTYLE		0b100000000
#define hasTITLE		0b1000000000
#define WH_DELAY		0b10000000000

//自动规则
struct V
{
	INT32 X;//X位置
	INT32 Y;//Y位置
	UINT32 cX;//宽
	UINT32 cY;//高
	DWORD Style;//基本样式
	DWORD ExStyle;//扩展样式
	INT16 Trans;//透明度
	UINT16 Flags;//各个属性的开关
	INT16 Delay;//延时ms数
	INT8 SizeOption;//ID_MENU_MAX:101、ID_MENU_NORMAL:102、ID_MENU_MIN:103、ID_MENU_HIDE:104
	bool EnableRule;//是否激活规则
	bool Topmost;//是否置顶
	bool Through;//是否点击穿透
	bool Active;//是否启用/禁用规则
	wchar_t NewTitle[MAX_PATH];//新标题
	wchar_t Title[MAX_PATH];//待匹配原标题
	wchar_t Path[MAX_PATH];//匹配路径
	wchar_t ClassName[MAX_PATH];//匹配窗口类名
};
//记录方式
struct AutoRuleV
{
	UINT16 index;//数字索引
	V *target;
};
//窗口链独立管理
struct ChainData
{
	HWND ListHwnd;
	int selfindex;
	bool type;//true为父子,false为主仆
	HWND Target;
};

//全局变量区
extern HWND g_MainHwnd;
extern HWND g_StaticClassname;
extern HWND g_StaticPid;
extern HWND g_EditTitlename;
extern HWND g_StaticExename;
extern HWND g_StaticStyle;
extern HWND g_StaticExStyle;
extern HWND g_EditTransparent;
extern HWND g_EditPosX;
extern HWND g_EditPosY;
extern HWND g_EditSizeX;
extern HWND g_EditSizeY;
extern HWND g_ButtonTransparent;
extern wchar_t g_ConfigPath[MAX_PATH];
extern UINT8 g_FrameWidth;
extern bool g_AutoRule;
extern int g_argc;
extern LPWSTR *g_argv;
extern unordered_map<UINT16, V *> Index啥;
extern unordered_map<wstring, vector<V *>> Class啥;
extern bool g_isTray;
extern vector<V> 啥;
extern UINT16 g_HotkeyShowMainwindow;
extern UINT16 g_HotkeySelect;
extern UINT16 g_HotkeySelectParent;
extern UINT16 g_HotkeyTransparent;
extern UINT16 g_HotkeyTopmost;
extern UINT16 g_HotkeyMove;
extern UINT16 g_HotkeySize;

//函数声明
wstring GetProcessNameFromWindow(HWND hwnd);
BOOL IsAdmin();
void ShakeWindow(HWND hWnd);
void SetWindowTransparent(HWND &target, UINT8 trans);
void FillInformation(HWND target);
void ResetInformation();
void ReadWriteConfig();
void LoadRules();
bool DeleteIniSection(const wchar_t *iniPath, const wchar_t *sectionToDelete);
void SaveRules(V *v, UINT16 i);
bool ModPair(UINT16 key);
