#include "标头.h"


//wstring GetProcessNameFromWindow2(HWND hwnd)
//{
//	DWORD processId = 0;
//
//	// 获取窗口的进程ID
//	GetWindowThreadProcessId(hwnd, &processId);
//
//	// 获取进程句柄
//	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
//	if(hProcess == NULL)
//	{
//		return L"无法打开进程";
//	}
//
//	// 获取进程路径
//	WCHAR processName[MAX_PATH];
//	DWORD max_path = MAX_PATH;
//	if(QueryFullProcessImageName(hProcess, 0, processName, &max_path))
//	{
//		CloseHandle(hProcess);
//		return processName;
//	}
//	else
//	{
//		CloseHandle(hProcess);
//		return L"无法获取进程名称";
//	}
//}

//利用窗口句柄获取程序名
wstring GetProcessNameFromWindow(HWND hwnd)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid); // 通过HWND获取进程PID
	if(pid == 0)
	{
		return L"";
	}

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if(!hProcess)
	{
		return L"";
	}

	TCHAR path[MAX_PATH] = { 0 };
	GetModuleFileNameEx(hProcess, NULL, path, MAX_PATH);
	CloseHandle(hProcess);
	return path;
}

//判断是否有管理员权限
BOOL IsAdmin()
{
	BOOL isAdmin = FALSE;
	PSID adminGroup = NULL;

	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if(AllocateAndInitializeSid(&NtAuthority, 2,
								SECURITY_BUILTIN_DOMAIN_RID,
								DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
	{

		CheckTokenMembership(NULL, adminGroup, &isAdmin);
		FreeSid(adminGroup);
	}

	return isAdmin;
}

//抖动一个窗口
void ShakeWindow(HWND hWnd)
{
	constexpr int offset = 10;       // 抖动幅度
	constexpr int times = 8;         // 抖动次数
	constexpr int delay = 20;        // 每次抖动的延迟(ms)

	RECT rect;
	GetWindowRect(hWnd, &rect);
	POINT pos = { rect.left,rect.top };
	HWND hParent = GetParent(hWnd);
	if(hParent && (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD))
	{
		ScreenToClient(hParent, &pos);
		for(int i = 0; i < times; i++)
		{
			// 左
			SetWindowPos(hWnd, NULL, pos.x - offset, pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 上
			SetWindowPos(hWnd, NULL, pos.x - offset, pos.y - offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 右
			SetWindowPos(hWnd, NULL, pos.x + offset, pos.y - offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 下
			SetWindowPos(hWnd, NULL, pos.x + offset, pos.y + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
		}
	}
	else
	{
		for(int i = 0; i < times; i++)
		{
			// 左
			SetWindowPos(hWnd, NULL, pos.x - offset, pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 上
			SetWindowPos(hWnd, NULL, pos.x - offset, pos.y - offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 右
			SetWindowPos(hWnd, NULL, pos.x + offset, pos.y - offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
			// 下
			SetWindowPos(hWnd, NULL, pos.x + offset, pos.y + offset, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			Sleep(delay);
		}


	}
	// 恢复原始位置
	SetWindowPos(hWnd, NULL, pos.x, pos.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

//设置窗口的透明度
bool SetWindowTransparent(HWND &target, INT32 &trans)
{
	DWORD exStyle = GetWindowLongPtr(target, GWL_EXSTYLE);
	SetWindowLongPtr(target, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
	if(GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd && trans <= 55)
	{
		MessageBox(g_MainHwnd, L"再透明我就要被弄丢了喵~", L">_<", MB_OK | MB_ICONINFORMATION);
		return false;
	}
	else
	{
		SetLayeredWindowAttributes(target, 0, trans, LWA_ALPHA);
		return true;
	}
}

//填充信息
void FillInformation(HWND target)
{
	wchar_t temp[MAX_PATH];
	//窗口类名
	GetClassName(target, temp, ARRAYSIZE(temp));
	SetWindowText(g_StaticClassname, temp);

	//窗口标题
	GetWindowText(target, temp, ARRAYSIZE(temp));
	SetWindowText(g_EditTitlename, temp);

	DWORD pid = 0;
	GetWindowThreadProcessId(target, &pid); // 通过窗口句柄获取进程PID
	


	//所属进程名
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	if(hProcess)
	{
		HMODULE hModule = NULL;
		DWORD cbNeeded = 0;
		EnumProcessModules(hProcess, &hModule, sizeof(hModule), &cbNeeded);
		GetModuleFileNameEx(hProcess, NULL, temp, MAX_PATH);
		CloseHandle(hProcess);
		wstring buf = wstr_format(L"%ws  PID:%lu", temp, pid);
		SetWindowText(g_StaticExename, buf.c_str());
		buf = wstr_format(L"%llu", (UINT64)hModule);
		SetWindowText(g_StaticInstance, buf.c_str());
	}
	else
	{
		SetWindowText(g_StaticExename, L"[操作错误]");
		SetWindowText(g_StaticInstance, L"[操作错误]");
	}

	//窗口风格
	DWORD Styles;
	string StyleName;
	Styles = GetWindowLongPtr(target, GWL_STYLE);
	StyleName = str_format("0x%08x ", Styles);
	for(auto &x : StyleFlags)
	{
		if(Styles & x.flag)
		{
			StyleName += x.name;
			StyleName += '|';
		}
	}
	StyleName.back() = '\0';
	SetWindowTextA(g_StaticStyle, StyleName.c_str());

	//窗口扩展风格
	Styles = GetWindowLongPtr(target, GWL_EXSTYLE);
	StyleName = str_format("0x%08x ", Styles);
	for(auto &x : ExStyleFlags)
	{
		if(Styles & x.flag)
		{

			StyleName += x.name;
			StyleName += '|';
		}
	}
	StyleName.back() = '\0';
	SetWindowTextA(g_StaticExStyle, StyleName.c_str());

	//透明度填充
	if(Styles & WS_EX_LAYERED)
	{
		BYTE trans;
		GetLayeredWindowAttributes(target, 0, &trans, NULL);
		wsprintf(temp, L"%d", trans);
		SetWindowText(g_EditTransparent, temp);
	}
	else
	{
		SetWindowText(g_EditTransparent, L"255");
	}

	//窗口位置大小
	RECT rect;
	GetWindowRect(target, &rect);
	wsprintf(temp, L"%ld", rect.left);
	SetWindowText(g_EditPosX, temp);
	wsprintf(temp, L"%ld", rect.top);
	SetWindowText(g_EditPosY, temp);
	wsprintf(temp, L"%ld", rect.right - rect.left);
	SetWindowText(g_EditSizeX, temp);
	wsprintf(temp, L"%ld", rect.bottom - rect.top);
	SetWindowText(g_EditSizeY, temp);
}

//清空信息
void ResetInformation()
{
	SetWindowText(g_EditPosX, NULL);
	SetWindowText(g_EditPosY, NULL);
	SetWindowText(g_EditSizeX, NULL);
	SetWindowText(g_EditSizeY, NULL);
	SetWindowText(g_StaticExename, NULL);
	SetWindowText(g_StaticInstance, NULL);
	SetWindowText(g_StaticClassname, NULL);
	SetWindowText(g_EditTitlename, NULL);
	SetWindowText(g_StaticStyle, NULL);
	SetWindowText(g_StaticExStyle, NULL);
	SetWindowText(g_EditTransparent, NULL);
}