#include "标头.h"
//全局变量区
HWND g_MainHwnd;
HINSTANCE g_hInstance;
TCHAR g_sTitle[] = TEXT("窗口把柄");
SIZE g_ClientSize;
SIZE g_FontSize;
int g_LineAnchorYStep;
INT g_LineAnchorY[10];
HFONT g_Font;
HFONT g_EditFont;
HWND g_ButtonSelect;
HWND g_ButtonShake;
HWND g_EditTransparent;
HWND g_ButtonTransparent;
HWND g_EditPosX;
HWND g_EditPosY;
HWND g_ButtonPos;
HWND g_EditSizeX;
HWND g_EditSizeY;
HWND g_ButtonSize;
HWND g_StaticExename;
HWND g_StaticPid;
HWND g_StaticClassname;
HWND g_EditTitlename;
HWND g_EditHandle;
HWND g_StaticStyle;
HWND g_StaticExStyle;
HWND g_ButtonEnable;
HWND g_ButtonTop;
HWND g_ButtonThrough;
HWND g_ButtonTitle;
HWND g_ButtonNoactivate;
LRESULT g_Noactivate;//存放窗口是否是不捕获焦点的状态，用于通知后续创建窗口也有相同属性
HWND g_ButtonStyle;
HWND g_StyleHWND = NULL;
HWND g_ExStyleHWND = NULL;
HWND g_ButtonExStyle;
HWND g_ButtonEnum;
HWND g_EnumHWND = NULL;
HWND g_ButtonEnumChild;
HWND g_EnumChildHWND = NULL;
HWND g_ButtonSetParent;
HWND g_SetParentHWND = NULL;
HWND g_ButtonMessage;
HWND g_MessageHWND = NULL;
HWND g_ButtonAbout;
HWND g_AboutHWND = NULL;
SIZE g_ScreenSize;//记录当前屏幕尺寸
HWND g_FrameHWND = NULL;
HWND g_ButtonAutoRule;
HWND g_AutoRuleHWND = NULL;
HWND g_AutoRuleDetailHWND = NULL;
HWND g_ButtonHotkey;
HWND g_HotkeyHWND = NULL;
NOTIFYICONDATA g_nid;//托盘数据结构体
UINT WM_KeepTray;//自定义保留托盘消息
wchar_t g_ConfigPath[MAX_PATH];
COLORREF g_FrameColor;//选择指示框颜色
UINT8 g_FrameWidth;//选择指示框宽度
bool g_AutoRule;//自动规则模式
HWINEVENTHOOK g_WindowHook;
HHOOK g_MouseHook;
unordered_map<UINT16, V*> Index啥;//从配置文件到数据库单项的映射
unordered_map<wstring, vector<V*>> Class啥;//从窗口类名到数据库多项组的映射
vector<V> 啥;//这是数据库
bool g_isTray;//决定程序初始时是否直接以托盘形式存在
int argc;
LPWSTR *argv;
UINT16 g_HotkeyShowMainwindow;
UINT16 g_HotkeySelect;
UINT16 g_HotkeySelectParent;
UINT16 g_HotkeyTransparent;//钩子，非传统快捷键
UINT16 g_HotkeyTopmost;
UINT16 g_HotkeyMove;//钩子，非传统快捷键
UINT16 g_HotkeySize;//钩子，非传统快捷键
POINT g_HotkeyLastPoint;//鼠标原点
RECT g_HotkeyLastWindow;//窗口原矩形
HWND g_HotkeyTargetWindow;//目标窗口句柄

#define HOOK SetWinEventHook(EVENT_OBJECT_CREATE, EVENT_OBJECT_CREATE,NULL,HandleWinEvent,0,0,WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

//检测鼠标事件，低级钩子，尽可能快返回
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#define break return CallNextHookEx(g_MouseHook, nCode, wParam, lParam)
	if(nCode != HC_ACTION)break;//只要普通消息
	switch(wParam)
	{
		case WM_MOUSEWHEEL:
		{
			//没有就算
			if(!g_HotkeyTransparent)break;
			//匹配修饰键
			if(!ModPair(g_HotkeyTransparent))break;
			//有普通键就匹配看看
			if(g_HotkeyTransparent >> 6)
			{
				if(!(GetAsyncKeyState(g_HotkeyTransparent >> 6) & 0x8000))
				{
					break;
				}
			}
			POINT pt;
			GetCursorPos(&pt);
			HWND target = GetAncestor(WindowFromPoint(pt), GA_ROOT);
			if(!IsWindow(target))break;
			
			int delta = GET_WHEEL_DELTA_WPARAM(((MSLLHOOKSTRUCT *)lParam)->mouseData);
			//获取原透明度
			UINT8 trans;
			if(GetWindowLongPtr(target, GWL_EXSTYLE) & WS_EX_LAYERED)
			{
				GetLayeredWindowAttributes(target, 0, &trans, NULL);
			}
			else
			{
				trans = 255;
			}
			//修改并5对齐
			if(delta > 0)//up
			{
				if(trans == 255)return 1;//过了也拦截，不能break
				trans = trans - trans % 5 + 5;
			}
			else//down
			{
				if(trans <= 5)return 1;
				if(trans % 5)
				{
					trans -= trans % 5;
				}
				else
				{
					trans -= 5;
				}
			}
			if(GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
			{
				if(trans <= 55)break;
			}
			SetWindowTransparent(target, trans);
			return 1;//拦截滚轮
		}
		case WM_LBUTTONDOWN:
		{
			//没有就算
			if(!g_HotkeyMove)break;
			//匹配修饰键
			if(!ModPair(g_HotkeyMove))break;
			//有普通键就匹配看看
			if(g_HotkeyMove >> 6)
			{
				if(!(GetAsyncKeyState(g_HotkeyMove >> 6) & 0x8000))
				{
					break;
				}
			}
			GetCursorPos(&g_HotkeyLastPoint);
			g_HotkeyTargetWindow = GetAncestor(WindowFromPoint(g_HotkeyLastPoint), GA_ROOT);
			if(!IsWindow(g_HotkeyTargetWindow))break;
			GetWindowRect(g_HotkeyTargetWindow, &g_HotkeyLastWindow);
			KillTimer(g_MainHwnd, ID_Timer_Size);//防止竞争
			SetTimer(g_MainHwnd, ID_Timer_Move, 16, NULL);
			return 1;//拦截左键按下
		}
		case WM_LBUTTONUP:
		{
			//没有就算
			if(!g_HotkeyMove)break;
			KillTimer(g_MainHwnd, ID_Timer_Move);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			//没有就算
			if(!g_HotkeySize)break;
			//匹配修饰键
			if(!ModPair(g_HotkeySize))break;
			//有普通键就匹配看看
			if(g_HotkeySize >> 6)
			{
				if(!(GetAsyncKeyState(g_HotkeySize >> 6) & 0x8000))
				{
					break;
				}
			}
			GetCursorPos(&g_HotkeyLastPoint);
			g_HotkeyTargetWindow = GetAncestor(WindowFromPoint(g_HotkeyLastPoint), GA_ROOT);
			if(!IsWindow(g_HotkeyTargetWindow))break;
			GetWindowRect(g_HotkeyTargetWindow, &g_HotkeyLastWindow);
			KillTimer(g_MainHwnd, ID_Timer_Move);//防止竞争
			SetTimer(g_MainHwnd, ID_Timer_Size, 16, NULL);
			return 1;//拦截右键按下
		}
		case WM_RBUTTONUP:
		{
			//没有就算
			if(!g_HotkeySize)break;
			KillTimer(g_MainHwnd, ID_Timer_Size);
			break;
		}
	}

#undef break
	return CallNextHookEx(g_MouseHook, nCode, wParam, lParam);
}

//判断窗口创建的回调函数
void CALLBACK HandleWinEvent(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	//多线程异步判断
	QueueUserWorkItem(
		[](LPVOID Param)->DWORD
	{
		HWND hwnd = (HWND)Param;
		wchar_t buf[MAX_PATH];
		GetClassName(hwnd, buf, ARRAYSIZE(buf));

		//没有该窗口类名，直接结束
		if(!Class啥.count(buf))return 0;

		for(const auto &x : Class啥[buf])
		{
			if(!x->Active)continue;//不启用就不判断了

			//匹配进程路径
			wchar_t temp[MAX_PATH];
			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
			if(!hProcess)continue;

#if _WIN32_WINNT > _WIN32_WINNT_WINXP
			DWORD dwPathNameSize = sizeof(temp);
			QueryFullProcessImageName(hProcess, 0, temp, &dwPathNameSize);
#else
			GetModuleFileNameEx(hProcess, NULL, temp, MAX_PATH);
#endif
			CloseHandle(hProcess);


			if(_wcsicmp(temp, x->Path) != 0)continue;

			//延时操作，保证各个情况都可用
			if(x->Flags & WH_DELAY)
			{
				Sleep(x->Delay);
			}

			//匹配窗口标题
			if(x->Flags & hasTITLE)
			{
				//最多等你100ms，怕是窗口标题不能立刻加载出来的
				for(int i = 0; i < 10; ++i)
				{
					GetWindowText(hwnd, temp, ARRAYSIZE(temp));
					if(temp[0] != L'\0')
					{
						break;
					}
					Sleep(10);
				}
				if(_wcsicmp(temp, x->Title) != 0)continue;
			}

			//设置位置
			if(x->Flags & WH_POS)
			{
				SetWindowPos(hwnd, NULL, x->X, x->Y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
			}

			//设置大小
			switch(x->SizeOption)
			{
				case ID_MENU_MAX:
				{
					ShowWindow(hwnd, SW_MAXIMIZE);
					break;
				}
				case ID_MENU_MIN:
				{
					ShowWindow(hwnd, SW_MINIMIZE);
					break;
				}
				case ID_MENU_HIDE:
				{
					ShowWindow(hwnd, SW_HIDE);
					break;
				}
			}
			if(x->Flags & WH_SIZE)
			{
				SetWindowPos(hwnd, NULL, 0, 0, x->cX, x->cY, SWP_NOMOVE | SWP_NOZORDER);
			}

			//设置启/禁用
			if(x->Flags & WH_ENABLE)
			{
				EnableWindow(hwnd, x->Enable);
			}

			//设置置顶
			if(x->Flags & WH_TOPMOST)
			{
				SetWindowPos(hwnd, x->Topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
			}	

			//设置标题
			if(x->Flags & WH_TITLE)
			{
				SetWindowText(hwnd, x->NewTitle);
			}

			//设置风格
			if(x->Flags & WH_STYLE)
			{
				SetWindowLongPtr(hwnd, GWL_STYLE, x->Style);
			}

			//设置扩展风格
			if(x->Flags & WH_EXSTYLE)
			{
				SetWindowLongPtr(hwnd, GWL_EXSTYLE, x->ExStyle);
			}

			//刷新窗口
			if((x->Flags & WH_TITLE) || (x->Flags & WH_STYLE) || (x->Flags & WH_EXSTYLE) || (x->Flags & WH_THROUGH))
			{
				InvalidateRect(hwnd, NULL, TRUE);
			}

			//设置透明度
			if(x->Flags & WH_TRANSPARENT)
			{
				if(x->Flags & WH_EXSTYLE)//假如同时还要修改的扩展样式
				{
					if(!(x->ExStyle & WS_EX_LAYERED))return 0;//扩展样式优先级更大 ，没有该项就无法修改，不改了
				}
				//最多尝试100次(1s)，改不成就算了
				for(int i = 0; i < 100; ++i)
				{
					DWORD exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
					SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
					SetLayeredWindowAttributes(hwnd, 0, x->Trans, LWA_ALPHA);

					BYTE curAlpha = 0;
					DWORD flags = GetLayeredWindowAttributes(hwnd, NULL, &curAlpha, &flags);
					if(flags & LWA_ALPHA && curAlpha == x->Trans)
					{
						break;
					}
					Sleep(10);

				}
			}

			//设置点击穿透
			if(x->Flags & WH_THROUGH)
			{
				//Sleep(666);
				SetWindowThrough(hwnd, x->Through, NULL);
			}
		}
		return 0;
	}, hwnd, WT_EXECUTELONGFUNCTION);

	return;
}

//选择按钮的左右键监听
LRESULT CALLBACK ButtonSelectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			//选择定时器都是互斥的，只能有一个
			KillTimer(g_MainHwnd, ID_Timer_RSelect);
			KillTimer(g_MainHwnd, ID_Timer_LSelectHotkey);
			KillTimer(g_MainHwnd, ID_Timer_RSelectHotkey);
			SetTimer(g_MainHwnd, ID_Timer_LSelect, 16, NULL);
			if(IsWindow(g_FrameHWND))
			{
				DestroyWindow(g_FrameHWND);
			}
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
				L"Window's Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				NULL,
				NULL, NULL, NULL
			);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			//选择定时器都是互斥的，只能有一个
			KillTimer(g_MainHwnd, ID_Timer_LSelect);
			KillTimer(g_MainHwnd, ID_Timer_LSelectHotkey);
			KillTimer(g_MainHwnd, ID_Timer_RSelectHotkey);
			SetTimer(g_MainHwnd, ID_Timer_RSelect, 16, NULL);
			if(IsWindow(g_FrameHWND))
			{
				DestroyWindow(g_FrameHWND);
			}
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_NOACTIVATE,
				L"Window's Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				NULL,
				NULL, NULL, NULL
			);
			break;
		}
	}
	// 调用默认的窗口过程
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//抖动按钮的左右键监听
LRESULT CALLBACK ButtonShakeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_RBUTTONDOWN:
		{
			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			HWND target = (HWND)atoi(handle);
			if(!IsWindow(target))
			{
				break;
			}
			if(IsWindow(g_FrameHWND))
			{
				DestroyWindow(g_FrameHWND);
			}
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
				L"Window's Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				g_MainHwnd,
				NULL, NULL, NULL
			);
			PostMessage(g_FrameHWND, WM_Frame, 0, (LPARAM)target);
			break;
		}
		case WM_RBUTTONUP:
		{
			DestroyWindow(g_FrameHWND);
			g_FrameHWND = NULL;
			break;
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//位置按钮的右键监听
LRESULT CALLBACK ButtonPosProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	static POINT cursorPos;
	static HWND target;
	switch(msg)
	{
		//发送定时器消息
		case WM_RBUTTONDOWN:
		{
			RECT rc;
			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			target = (HWND)atoi(handle);
			if(!IsWindow(target))
			{
				break;
			}
			GetWindowRect(target, &rc);
			GetCursorPos(&cursorPos);
			SetCursorPos(rc.left, rc.top);
			SetCapture(hwnd);
			SetTimer(g_MainHwnd, ID_Timer_Pos, 16, NULL);
			break;
		}
		//停止定时器并更新信息
		case WM_RBUTTONUP:
		{
			if(!IsWindow(target))
			{
				break;
			}
			KillTimer(g_MainHwnd, 1);
			SetCursorPos(cursorPos.x, cursorPos.y);
			ReleaseCapture();
			char handle[HANDLESIZE];
			wchar_t Temp[21];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			RECT rect;
			GetWindowRect((HWND)atoi(handle), &rect);
			wsprintf(Temp, L"%ld", rect.left);
			SetWindowText(g_EditPosX, Temp);
			wsprintf(Temp, L"%ld", rect.top);
			SetWindowText(g_EditPosY, Temp);
			break;
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//大小按钮的右键监听
LRESULT CALLBACK ButtonSizeProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_RBUTTONUP:
		{
			HMENU hPopupMenu = CreatePopupMenu();
			if(!hPopupMenu) break;

			// 根据 status 添加第一个项
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MAX, L"最大化");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_NORMAL, L"正常");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MIN, L"最小化");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_HIDE, L"隐藏");
			// 获取鼠标位置（屏幕坐标）
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			// 显示菜单
			TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, g_MainHwnd, NULL);

			DestroyMenu(hPopupMenu); // 别忘了销毁
			break;
		}
	}
	// 调用默认的窗口过程
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//不透明度按钮的滚轮监听
LRESULT CALLBACK ButtonTransparentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			HWND target = (HWND)atoi(handle);
			if(!IsWindow(target))
			{
				break;
			}
			//获取原透明度
			UINT8 trans;
			if(GetWindowLongPtr(target, GWL_EXSTYLE) & WS_EX_LAYERED)
			{
				GetLayeredWindowAttributes(target, 0, &trans, NULL);
			}
			else
			{
				trans = 255;
			}
			//修改并5对齐
			if(delta > 0)//up
			{
				if(trans == 255)
				{
					break;
				}
				trans = trans - trans % 5 + 5;
			}
			else//down
			{
				if(trans == 0)
				{
					break;
				}
				if(trans % 5)
				{
					trans -= trans % 5;
				}
				else
				{
					trans -= 5;
				}
			}

			if(GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
			{
				if(trans <= 55)
				{
					MessageBox(g_MainHwnd, L"再透明我就要被弄丢了喵~", L">_<", MB_OK | MB_ICONINFORMATION);
					break;
				}
			}
			SetWindowTransparent(target, trans);
			snprintf(handle, ARRAYSIZE(handle), "%d", trans);//handle变量复用，存放数值
			SetWindowTextA(g_EditTransparent, handle);//handle变量复用，存放数值
			break;
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//窗口列表按钮右键监听
LRESULT CALLBACK ButtonEnumProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_RBUTTONUP:
		{
			SendMessage(g_MainHwnd, WM_COMMAND, MAKEWPARAM(ID_BTN_EnumChild, 0), 0);
			break;
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

//自动规则按钮右键监听
LRESULT CALLBACK ButtonAutoRuleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_RBUTTONUP:
		{
			HMENU hPopupMenu = CreatePopupMenu();
			if(!hPopupMenu) break;

			// 根据 status 添加第一个项
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_AUTORULEENABLE, L"启用");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_AUTORULEDISENABLE, L"禁用");
			CheckMenuItem(hPopupMenu, g_AutoRule ? ID_MENU_AUTORULEENABLE : ID_MENU_AUTORULEDISENABLE, MF_BYCOMMAND | MF_CHECKED);
			// 获取鼠标位置（屏幕坐标）
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			// 显示菜单
			TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, GetParent(hwnd), NULL);

			DestroyMenu(hPopupMenu); // 别忘了销毁
			break;
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}



//窗口风格窗口过程
LRESULT CALLBACK StyleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditStyle;
	static HWND ButtonApplyStyle;
	static DWORD StyleValue = 0;
	static HWND target;
	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);
			EditStyle = CreateWindowEx(
				NULL,
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				g_FontSize.cx,
				g_FontSize.cy * 0.5,
				g_FontSize.cx * 13,
				g_FontSize.cy * 1.2,
				hwnd,
				NULL,
				NULL,
				NULL
			);
			SendMessage(EditStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);
			
			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			target = (HWND)atoi(handle);

			StyleValue = GetWindowLongPtr(target, GWL_STYLE);
			wchar_t styletext[12];
			_snwprintf(styletext, ARRAYSIZE(styletext), L"0x%08x", StyleValue);
			SetWindowText(EditStyle, styletext);
			int i = g_FontSize.cy * 2;
			for(auto &x : StyleFlags)
			{
				HWND temp = CreateWindowA(
					"button",
					x.name,
					WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
					g_FontSize.cx * 2,
					i,
					g_FontSize.cx * 11,
					g_FontSize.cy * 1.2,
					hwnd,
					(HMENU)1145,
					NULL,
					NULL);
				SendMessage(temp, WM_SETFONT, (WPARAM)g_EditFont, 0);
				_snwprintf(styletext, ARRAYSIZE(styletext), L"0x%08x", x.flag);
				CreatePointToolTip(hwnd, temp, styletext, NULL, false, TTI_NONE, NULL, NULL);
				if(StyleValue & x.flag || x.flag == 0)
				{
					SendMessage(temp, BM_SETCHECK, BST_CHECKED, 0);
				}
				SetWindowLongPtr(temp, GWLP_USERDATA, (DWORD)x.flag);

				i += g_FontSize.cy * 1.2;
			}
			ButtonApplyStyle = CreateWindowEx(
				NULL,
				L"button",
				L"应用",
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				11 * g_FontSize.cx,
				i,
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.5,
				hwnd,
				(HMENU)9191, NULL, NULL
			);
			SendMessage(ButtonApplyStyle, WM_SETFONT, (WPARAM)g_Font, 0);
			break;
		}
		case WM_COMMAND:
		{
			if(HIWORD(wParam) != BN_CLICKED)//仅处理点击事件
			{
				break;
			}
			if(LOWORD(wParam) == 1145)
			{
				bool selected = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				if(selected)
				{
					StyleValue |= (DWORD)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				}
				else
				{
					StyleValue &= ~(DWORD)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				}
				char TextStyleValue[12];
				sprintf(TextStyleValue, "0x%08x", StyleValue);
				SetWindowTextA(EditStyle, TextStyleValue);
			}
			else
			{
				SetWindowLongPtr(target, GWL_STYLE, StyleValue);
				FillInformation(target);
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
		}
		case WM_CTLCOLORBTN:
		{
			HDC hdcButton = (HDC)wParam;
			SetBkMode(hdcButton, TRANSPARENT);
			return (LRESULT)GetStockObject(HOLLOW_BRUSH);
		}
		case WM_DESTROY:
		{
			EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//扩展窗口风格窗口过程
LRESULT CALLBACK ExStyleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditExStyle;
	static HWND ButtonApplyStyle;
	static DWORD ExStyleValue = 0;
	static HWND target;
	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);
			EditExStyle = CreateWindowEx(
				NULL,
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				g_FontSize.cx,
				g_FontSize.cy * 0.5,
				g_FontSize.cx * 15,
				g_FontSize.cy * 1.2,
				hwnd,
				NULL,
				NULL,
				NULL
			);
			SendMessage(EditExStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			target = (HWND)atoi(handle);
			
			ExStyleValue = GetWindowLongPtr(target, GWL_EXSTYLE);
			wchar_t styletext[12];
			_snwprintf(styletext, ARRAYSIZE(styletext), L"0x%08x", ExStyleValue);
			SetWindowText(EditExStyle, styletext);
			int i = g_FontSize.cy * 2;
			for(auto &x : ExStyleFlags)
			{
				HWND temp = CreateWindowA(
					"button",
					x.name,
					WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
					g_FontSize.cx,
					i,
					g_FontSize.cx * 15,
					g_FontSize.cy * 1.2,
					hwnd,
					(HMENU)1145,
					NULL,
					NULL);
				SendMessage(temp, WM_SETFONT, (WPARAM)g_EditFont, 0);
				_snwprintf(styletext, ARRAYSIZE(styletext), L"0x%08x", x.flag);
				CreatePointToolTip(hwnd, temp, styletext, NULL, false, TTI_NONE, NULL, NULL);
				if(ExStyleValue & x.flag || x.flag == 0)
				{
					SendMessage(temp, BM_SETCHECK, BST_CHECKED, 0);
				}
				SetWindowLongPtr(temp, GWLP_USERDATA, (DWORD)x.flag);

				i += g_FontSize.cy * 1.2;
			}
			ButtonApplyStyle = CreateWindowEx(
				NULL,
				L"button",
				L"应用",
				WS_VISIBLE | WS_CHILD | WS_BORDER,
				13 * g_FontSize.cx,
				i,
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.5,
				hwnd,
				(HMENU)9191, NULL, NULL
			);
			SendMessage(ButtonApplyStyle, WM_SETFONT, (WPARAM)g_Font, 0);
			break;
		}
		case WM_COMMAND:
		{
			if(HIWORD(wParam) != BN_CLICKED)//点击事件
			{
				break;
			}
			if(LOWORD(wParam) == 1145)
			{
				bool selected = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				if(selected)
				{
					ExStyleValue |= (DWORD)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				}
				else
				{
					ExStyleValue &= ~(DWORD)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
				}
				char TextStyleValue[12];
				sprintf(TextStyleValue, "0x%08x", ExStyleValue);
				SetWindowTextA(EditExStyle, TextStyleValue);
			}
			else
			{
				SetWindowLongPtr(target, GWL_EXSTYLE, ExStyleValue);
				FillInformation(target);
				SendMessage(hwnd, WM_CLOSE, 0, 0);
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//枚举顶级窗口过程
LRESULT CALLBACK EnumProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND ListHwnd;
	static INT SortIndex;
	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);
			ListHwnd = CreateWindowW(
				WC_LISTVIEW,
				NULL,
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
				g_FontSize.cx * 0,
				g_FontSize.cy * 0,
				g_FontSize.cx * 58,
				g_FontSize.cy * 38,
				hwnd,
				NULL,NULL, NULL);
			ListView_SetExtendedListViewStyle(ListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(ListHwnd, WM_SETFONT, (WPARAM)g_EditFont, 0);

			LVCOLUMNW lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

			lvc.iSubItem = 0;
			lvc.pszText = L"窗口句柄";
			lvc.cx = g_FontSize.cx * 5;
			ListView_InsertColumn(ListHwnd, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = L"窗口标题";
			lvc.cx = g_FontSize.cx * 5 * 2;
			ListView_InsertColumn(ListHwnd, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = L"窗口类名";
			lvc.cx = g_FontSize.cx * 5 * 2;
			ListView_InsertColumn(ListHwnd, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = L"进程名";
			lvc.cx = g_FontSize.cx * 4 * 3;
			ListView_InsertColumn(ListHwnd, 3, &lvc);

			lvc.iSubItem = 4;
			lvc.pszText = L"PID";
			lvc.cx = g_FontSize.cx * 3;
			ListView_InsertColumn(ListHwnd, 4, &lvc);

			lvc.iSubItem = 5;
			lvc.pszText = L"可见性";
			lvc.cx = g_FontSize.cx * 4;
			ListView_InsertColumn(ListHwnd, 5, &lvc);

			lvc.iSubItem = 6;
			lvc.pszText = L"拥有窗口";
			lvc.cx = g_FontSize.cx * 5;
			ListView_InsertColumn(ListHwnd, 6, &lvc);

			lvc.iSubItem = 7;
			lvc.pszText = L"顶级拥有窗口";
			lvc.cx = g_FontSize.cx * 7;
			ListView_InsertColumn(ListHwnd, 7, &lvc);

			auto EnumWindowsProc = [](HWND hwnd, LPARAM lParam)->BOOL
			{
				wchar_t buf[MAX_PATH];
				_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)hwnd);

				//0窗口句柄
				LVITEMW lvi;
				lvi.mask = LVIF_TEXT;
				lvi.iItem = 0;
				lvi.iSubItem = 0;
				lvi.pszText = buf;
				ListView_InsertItem(ListHwnd, &lvi);

				//1窗口标题
				GetWindowTextW(hwnd, buf, ARRAYSIZE(buf));
				ListView_SetItemText(ListHwnd, 0, 1, buf);

				//2窗口类名
				GetClassName(hwnd, buf, ARRAYSIZE(buf));
				ListView_SetItemText(ListHwnd, 0, 2, buf);

				//4PID
				DWORD pid = 0;
				GetWindowThreadProcessId(hwnd, &pid);
				_snwprintf(buf, ARRAYSIZE(buf), L"%lu", pid);
				ListView_SetItemText(ListHwnd, 0, 4, buf);

				//3进程名
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
				buf[0] = L'\0';
				if(hProcess)
				{
					GetModuleFileNameEx(hProcess, NULL, buf, MAX_PATH);
					CloseHandle(hProcess);
					wstring temp = buf;
					size_t pos = temp.find_last_of(L"\\/");
					if(pos != wstring::npos)
					{
						WstringToWchar(temp.substr(pos + 1), buf);
					}
				}
				ListView_SetItemText(ListHwnd, 0, 3, buf);

				//5可见性
				if(IsWindowVisible(hwnd))
				{
					ListView_SetItemText(ListHwnd, 0, 5, L"是");
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 5, L"否");
				}

				HWND ご主人様;
				//6拥有窗口
				if(ご主人様 = GetWindow(hwnd, GW_OWNER))
				{
					_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)ご主人様);
					ListView_SetItemText(ListHwnd, 0, 6, buf);
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 6, L"无");
				}
				//7顶级拥有窗口
				if((ご主人様 = GetAncestor(hwnd, GA_ROOTOWNER)) != hwnd)
				{
					_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)ご主人様);
					ListView_SetItemText(ListHwnd, 0, 7, buf);
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 7, L"无");
				}
				return true; //返回true继续枚举,false停止
			};

			EnumWindows(EnumWindowsProc, 0);
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR phdr = (LPNMHDR)lParam;
			if(phdr->hwndFrom == ListHwnd)
			{
				if(phdr->code == LVN_COLUMNCLICK)
				{

					NMLISTVIEW *pnmv = (NMLISTVIEW *)lParam;
					int iColumn = pnmv->iSubItem;  // 哪一列被点了

					SortListAndUpdateArrow(ListHwnd, SortIndex, iColumn);
				}
				else if(phdr->code == NM_DBLCLK)
				{
					LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
					int iRow = pia->iItem;     // 双击的行
					//int iCol = pia->iSubItem;  // 双击的列（如果点击到具体列）

					if(iRow >= 0)
					{
						wchar_t buf[MAX_PATH];
						ListView_GetItemText(ListHwnd, iRow, 0, buf, 256);
						SetWindowText(g_EditHandle, buf);
						HWND target = (HWND)_wtoi(buf);
						FillInformation(target);
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//枚举子窗口过程
LRESULT CALLBACK EnumChildWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND ListHwnd;
	static INT SortIndex;
	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);
			ListHwnd = CreateWindowW(
				WC_LISTVIEW,
				NULL,
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
				g_FontSize.cx * 0,
				g_FontSize.cy * 0,
				g_FontSize.cx * 54,
				g_FontSize.cy * 38,
				hwnd,
				NULL, NULL, NULL);
			ListView_SetExtendedListViewStyle(ListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(ListHwnd, WM_SETFONT, (WPARAM)g_EditFont, 0);

			LVCOLUMNW lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

			lvc.iSubItem = 0;
			lvc.pszText = L"窗口句柄";
			lvc.cx = g_FontSize.cx * 5;
			ListView_InsertColumn(ListHwnd, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = L"窗口标题";
			lvc.cx = g_FontSize.cx * 5 * 2;
			ListView_InsertColumn(ListHwnd, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = L"窗口类名";
			lvc.cx = g_FontSize.cx * 5 * 2;
			ListView_InsertColumn(ListHwnd, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = L"进程名";
			lvc.cx = g_FontSize.cx * 4 * 3;
			ListView_InsertColumn(ListHwnd, 3, &lvc);

			lvc.iSubItem = 4;
			lvc.pszText = L"PID";
			lvc.cx = g_FontSize.cx * 3;
			ListView_InsertColumn(ListHwnd, 4, &lvc);

			lvc.iSubItem = 5;
			lvc.pszText = L"可见性";
			lvc.cx = g_FontSize.cx * 4;
			ListView_InsertColumn(ListHwnd, 5, &lvc);

			lvc.iSubItem = 6;
			lvc.pszText = L"父窗口";
			lvc.cx = g_FontSize.cx * 4;
			ListView_InsertColumn(ListHwnd, 6, &lvc);

			lvc.iSubItem = 7;
			lvc.pszText = L"顶级父窗口";
			lvc.cx = g_FontSize.cx * 6;
			ListView_InsertColumn(ListHwnd, 7, &lvc);
			char handle[HANDLESIZE];
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			HWND target = (HWND)atoi(handle);

			auto EnumChildProc = [](HWND hwnd, LPARAM lParam)->BOOL
			{
				wchar_t buf[MAX_PATH];
				_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)hwnd);
				HWND ListHwnd = *(HWND *)lParam;

				//0窗口句柄
				LVITEMW lvi;
				lvi.mask = LVIF_TEXT;
				lvi.iItem = 0;
				lvi.iSubItem = 0;
				lvi.pszText = buf;
				ListView_InsertItem(ListHwnd, &lvi);

				//1窗口标题
				GetWindowTextW(hwnd, buf, ARRAYSIZE(buf));
				ListView_SetItemText(ListHwnd, 0, 1, buf);

				//2窗口类名
				GetClassName(hwnd, buf, ARRAYSIZE(buf));
				ListView_SetItemText(ListHwnd, 0, 2, buf);

				//4PID
				DWORD pid = 0;
				GetWindowThreadProcessId(hwnd, &pid);
				_snwprintf(buf, ARRAYSIZE(buf), L"%lu", pid);
				ListView_SetItemText(ListHwnd, 0, 4, buf);

				//3进程名
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
				buf[0] = L'\0';
				if(hProcess)
				{
					GetModuleFileNameEx(hProcess, NULL, buf, MAX_PATH);
					CloseHandle(hProcess);
					wstring temp = buf;
					size_t pos = temp.find_last_of(L"\\/");
					if(pos != wstring::npos)
					{
						WstringToWchar(temp.substr(pos + 1), buf);
					}
				}
				ListView_SetItemText(ListHwnd, 0, 3, buf);

				//5可见性
				if(IsWindowVisible(hwnd))
				{
					ListView_SetItemText(ListHwnd, 0, 5, L"是");
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 5, L"否");
				}

				HWND ご主人様;

				//6父窗口
				if(ご主人様 = GetParent(hwnd))
				{
					_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)ご主人様);
					ListView_SetItemText(ListHwnd, 0, 6, buf);
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 6, L"无");
				}

				//7顶级父窗口
				if((ご主人様 = GetAncestor(hwnd, GA_ROOT)) != hwnd)
				{
					_snwprintf(buf, ARRAYSIZE(buf), L"%llu", (UINT64)ご主人様);
					ListView_SetItemText(ListHwnd, 0, 7, buf);
				}
				else
				{
					ListView_SetItemText(ListHwnd, 0, 7, L"无");
				}
				return true; //返回true继续枚举,false停止
			};

			EnumChildWindows(target, EnumChildProc, (LPARAM)&ListHwnd);
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR phdr = (LPNMHDR)lParam;
			if(phdr->hwndFrom == ListHwnd)
			{
				if(phdr->code == LVN_COLUMNCLICK)
				{

					NMLISTVIEW *pnmv = (NMLISTVIEW *)lParam;
					int iColumn = pnmv->iSubItem;  // 哪一列被点了

					SortListAndUpdateArrow(ListHwnd, SortIndex, iColumn);
				}
				else if(phdr->code == NM_DBLCLK)
				{
					LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
					int iRow = pia->iItem;     // 双击的行
					//int iCol = pia->iSubItem;  // 双击的列（如果点击到具体列）

					if(iRow >= 0)
					{
						wchar_t buf[MAX_PATH];
						ListView_GetItemText(ListHwnd, iRow, 0, buf, MAX_PATH);
						SetWindowText(g_EditHandle, buf);
						HWND target = (HWND)_wtoi(buf);
						FillInformation(target);
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//窗口归属窗口过程
LRESULT CALLBACK SetParentProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditTarget;
	static HWND EditNewParent;
	static HWND ButtonApply;
	char handle[HANDLESIZE];
	HWND target;
	HWND parent;
	switch(msg)
	{
		case WM_CREATE:
		{
			EditTarget = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[0],
				g_FontSize.cx * 7,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditTarget, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditNewParent = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[1],
				g_FontSize.cx * 7,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditNewParent, WM_SETFONT, (WPARAM)g_EditFont, 0);

			//快捷填充
			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			target = (HWND)atoi(handle);
			if(IsWindow(target))
			{
				SetWindowTextA(EditTarget, handle);
			}

			ButtonApply = CreateWindow(
				L"button",
				L"应用",
				WS_VISIBLE | WS_CHILD,
				12 * g_FontSize.cx,
				g_LineAnchorY[2],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)114,
				NULL, NULL
			);
			SendMessage(ButtonApply, WM_SETFONT, (WPARAM)g_EditFont, 0);

			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			HGDIOBJ OldObj = SelectObject(hdc, g_Font);

			RECT RectDrawText;
			RectDrawText.left = 1 * g_FontSize.cx;
			RectDrawText.right = 6 * g_FontSize.cx;
			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("目标窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("新父窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);


			SelectObject(hdc, OldObj);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case 114:
				{
					GetWindowTextA(EditTarget, handle, ARRAYSIZE(handle));
					target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						MessageBox(hwnd, L"目标窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					if(GetAncestor(target, GA_ROOT) == g_MainHwnd || GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}

					GetWindowTextA(EditNewParent, handle, ARRAYSIZE(handle));
					parent = (HWND)atoi(handle);
					if(parent == NULL)
					{
						DWORD style = GetWindowLongPtr(target, GWL_STYLE);
						style &= ~WS_CHILD;
						SetWindowLongPtr(target, GWL_STYLE, style);
						SetParent(target, NULL);
					}
					else if(!IsWindow(parent))
					{
						MessageBox(hwnd, L"新父窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					else
					{
						DWORD style = GetWindowLongPtr(target, GWL_STYLE);
						style |= WS_CHILD;
						SetWindowLongPtr(target, GWL_STYLE, style);
						SetParent(target, parent);
					}
					break;
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			g_SetParentHWND = NULL;
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//发送信息窗口过程
LRESULT CALLBACK MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND target;
	char handle[HANDLESIZE];//note：为什么之前有+1？
	static HWND EditTarget;
	static HWND ButtonSendType1;
	static HWND ButtonSendType2;
	static HWND EditMessage;
	static HWND EditwParam;
	static HWND EditlParam;
	static HWND ButtonApply;
	switch(msg)
	{
		case WM_CREATE:
		{
			ButtonSendType1 = CreateWindow(
				L"button",
				L"PostMessage",
				WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON | WS_GROUP,
				6 * g_FontSize.cx,
				g_LineAnchorY[0],
				7.5 * g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(ButtonSendType1, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SendMessage(ButtonSendType1, BM_SETCHECK, BST_CHECKED, 0);

			ButtonSendType2 = CreateWindow(
				L"button",
				L"SendMessage",
				WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
				14 * g_FontSize.cx,
				g_LineAnchorY[0],
				7.5 * g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(ButtonSendType2, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditTarget = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[1],
				g_FontSize.cx * 12,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditTarget, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditMessage = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[2],
				g_FontSize.cx * 12,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditMessage, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditwParam = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[3],
				g_FontSize.cx * 12,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditwParam, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditlParam = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				7 * g_FontSize.cx,
				g_LineAnchorY[4],
				g_FontSize.cx * 12,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditlParam, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonApply = CreateWindow(
				L"button",
				L"发送",
				WS_VISIBLE | WS_CHILD,
				18 * g_FontSize.cx,
				g_LineAnchorY[5],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)114,
				NULL, NULL
			);
			SendMessage(ButtonApply, WM_SETFONT, (WPARAM)g_EditFont, 0);

			GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
			target = (HWND)atoi(handle);
			if(IsWindow(target))
			{
				SetWindowTextA(EditTarget, handle);
			}
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			HGDIOBJ OldObj = SelectObject(hdc, g_Font);

			RECT RectDrawText;
			RectDrawText.left = 1 * g_FontSize.cx;
			RectDrawText.right = 6 * g_FontSize.cx;
			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("发送方式"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("目标窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[2];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("消息ID"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[3];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("wParam"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[4];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("lParam"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			SelectObject(hdc, OldObj);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case 114:
				{
					GetWindowTextA(EditTarget, handle, ARRAYSIZE(handle));
					target = (HWND)strtoll(handle, NULL, 10);
					if(!IsWindow(target))
					{
						MessageBox(hwnd, L"目标窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					else if(GetAncestor(target, GA_ROOT) == g_MainHwnd || GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}
					UINT message;
					GetWindowTextA(EditMessage, handle, ARRAYSIZE(handle));
					message = strtoll(handle, NULL, 10);
					WPARAM wparam;
					GetWindowTextA(EditwParam, handle, ARRAYSIZE(handle));
					wparam = strtoll(handle, NULL, 10);
					LPARAM lparam;
					GetWindowTextA(EditlParam, handle, ARRAYSIZE(handle));
					lparam = strtoll(handle, NULL, 10);
					if(SendMessage(ButtonSendType1, BM_GETCHECK, 0, 0) == BST_CHECKED)
					{
						PostMessage(target, message, wparam, lparam);
					}
					else
					{
						DWORD_PTR result = 0;
						if(!SendMessageTimeout(target, message, wparam, lparam,SMTO_ABORTIFHUNG, 3000, (PDWORD_PTR)&result))
						{
							wchar_t temp[MAX_PATH];
							_snwprintf(temp, ARRAYSIZE(temp), L"函数失败或超时（3秒），返回值:%llu", (INT64)result);
							MessageBox(hwnd, temp, L">_<", MB_OK | MB_ICONERROR);
						}
					}
					break;
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			g_MessageHWND = NULL;
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//关于窗口过程
LRESULT CALLBACK AboutProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HICON hIcon;
	static HWND LinkHwnd_Home;
	static HWND LinkHwnd_Patron;
	static HWND LinkHwnd_Github;
	static HFONT BoldFont;
	static float velocity;
	static float horizon_velocity;
	static RECT desktopRect;
	constexpr int ID_LINK = 114;//链接ID
	constexpr float gravity = 0.5;//重力加速度
	static POINT lastPos;
	static DWORD lastTime;
	switch(msg)
	{
		case WM_CREATE:
		{
			hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR);
			BoldFont = CreateFont(
				-g_ClientSize.cy / 23,
				0,
				0,
				0,
				FW_BOLD,
				false,
				false,
				false,
				GB2312_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				FF_SWISS,
				TEXT("微软雅黑")
			);
			LinkHwnd_Home = CreateWindowEx(
				NULL, L"SysLink",
				L"本软件由 <a href=\"https://space.bilibili.com/1081364881\">Bilibili-个人隐思</a> 用心打造",
				WS_CHILD | WS_VISIBLE,
				g_FontSize.cx * 2,
				g_LineAnchorY[3] + g_FontSize.cy,
				g_ClientSize.cx,
				g_FontSize.cy * 1.5,
				hwnd,
				(HMENU)ID_LINK,
				NULL,
				NULL
			);
			SendMessage(LinkHwnd_Home, WM_SETFONT, (WPARAM)g_Font, 0);

			LinkHwnd_Patron = CreateWindowEx(
				NULL, L"SysLink",
				L"如果觉得对您有所帮助，不妨在 <a href=\"https://afdian.com/a/X1415\">爱发电</a> 赞助一下我，以支持我做出更多优质作品",
				WS_CHILD | WS_VISIBLE,
				g_FontSize.cx * 2,
				g_LineAnchorY[4] + g_FontSize.cy,
				g_ClientSize.cx,
				g_FontSize.cy * 1.5,
				hwnd,
				(HMENU)ID_LINK,
				NULL,
				NULL
			);
			SendMessage(LinkHwnd_Patron, WM_SETFONT, (WPARAM)g_Font, 0);

			LinkHwnd_Github = CreateWindowEx(
				NULL, L"SysLink",
				L"Github开源地址: <a href=\"https://github.com/SiyuanX237/Window-s-Handle\">Window's Handle</a>",
				WS_CHILD | WS_VISIBLE,
				g_FontSize.cx * 2,
				g_LineAnchorY[5] + g_FontSize.cy,
				g_ClientSize.cx,
				g_FontSize.cy * 1.5,
				hwnd,
				(HMENU)ID_LINK,
				NULL,
				NULL
			);
			SendMessage(LinkHwnd_Github, WM_SETFONT, (WPARAM)g_Font, 0);

			GetClientRect(GetDesktopWindow(), &desktopRect);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			HGDIOBJ OldFont = SelectObject(hdc, BoldFont);
			DrawIconEx(hdc, g_FontSize.cx * 4, g_LineAnchorY[0], hIcon, g_FontSize.cx * 5, g_FontSize.cx * 5, 0, NULL, DI_NORMAL);
			RECT RectDrawText;
			RectDrawText.left = g_FontSize.cx * 15;
			RectDrawText.right = g_FontSize.cx * 40;

			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_FontSize.cy * 2;
			DrawText(hdc, TEXT("Window's Handle - 窗口把柄"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);


			SelectObject(hdc, g_Font);
			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_FontSize.cy * 1.3;
			DrawText(hdc, TEXT("版本：1.3.0.0                日期：2025.11.2"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);
			MoveToEx(hdc, g_FontSize.cx * 2, g_LineAnchorY[2], NULL);
			LineTo(hdc, g_FontSize.cx * 38, g_LineAnchorY[2]);

			RectDrawText.left = g_FontSize.cx * 2;
			RectDrawText.right = g_ClientSize.cx - g_FontSize.cx * 2;
			RectDrawText.top = g_LineAnchorY[2] + g_FontSize.cy;
			RectDrawText.bottom = RectDrawText.top + g_FontSize.cy * 2;
			DrawText(hdc, TEXT("Window's Handle是个免费开源软件，您可以随意下载传播与使用"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);
			SelectObject(hdc, OldFont);
			EndPaint(hwnd, &ps);

			break;
		}
		case WM_TIMER:
		{
			RECT winRect;
			GetWindowRect(hwnd, &winRect);
			int height = winRect.bottom - winRect.top;
			int width = winRect.right - winRect.left;

			int newY = winRect.top + velocity;
			int newX = winRect.left + horizon_velocity;

			// 模拟屏幕底部碰撞
			if(newY + height > desktopRect.bottom)
			{
				newY = desktopRect.bottom - height;
				velocity = -(velocity / 1.5);
				if(velocity< 4 && velocity > -4)
				{
					velocity = 0;
					horizon_velocity = 0;//清零
					KillTimer(hwnd, 1); // 停止重力
					break;
				}
			}
			else if(newY < 0)
			{
				newY = 0;
				velocity = -velocity;
			}
			else
			{
				velocity += gravity;
			}

			if(newX <= 0)
			{
				newX = 0;
				horizon_velocity = -(horizon_velocity * 0.4);
			}
			else if(newX + width >= desktopRect.right)
			{
				newX = desktopRect.right - width;
				horizon_velocity = -(horizon_velocity * 0.4);
			}
			MoveWindow(hwnd, newX, newY, width, height, true);
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			GetCursorPos(&lastPos);
			KillTimer(hwnd, ID_Timer_Pos);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			SetTimer(hwnd, 1, 8, NULL);//开始重力
			break;
		}
		case WM_MOVING:
		{
			RECT *pRect = (RECT *)lParam;
			POINT currPos = { pRect->left, pRect->top };

			DWORD now = GetTickCount();

			DWORD deltaTime = now - lastTime;
			if(deltaTime > 0)
			{
				int dx = currPos.x - lastPos.x;
				int dy = currPos.y - lastPos.y;
				horizon_velocity = (double)dx / deltaTime * 32; //帧速
				velocity = (double)dy / deltaTime * 32; //帧速
			}

			lastPos = currPos;
			lastTime = now;
			break;
		}
		case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS *wp = (WINDOWPOS *)lParam;
			wp->flags |= SWP_NOSIZE;
			break;
		}
		case WM_CTLCOLORSTATIC:
		{
			HDC hdcStatic = (HDC)wParam;
			HWND hwndStatic = (HWND)lParam;
			return (INT_PTR)GetStockBrush(WHITE_BRUSH);
		}
		case WM_NOTIFY:
		{
			LPNMHDR lpnmh = (LPNMHDR)lParam;
			if(lpnmh->code == NM_CLICK || lpnmh->code == NM_RETURN)
			{
				if(lpnmh->idFrom == ID_LINK) // ID 匹配
				{
					NMLINK *pNMLink = (NMLINK *)lParam;
					ShellExecute(NULL, L"open", pNMLink->item.szUrl, NULL, NULL, SW_NORMAL);
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			DestroyIcon(hIcon);
			DeleteObject(BoldFont);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//选择窗口时显示一个框框
LRESULT CALLBACK FrameProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND Target = NULL;
	static HBRUSH hBrBack;
	static HPEN hPen;
	switch(msg)
	{
		case WM_CREATE:
		{
			hBrBack = CreateSolidBrush(0x114514);
			hPen = CreatePen(PS_SOLID, g_FrameWidth, g_FrameColor);
			SetLayeredWindowAttributes(hwnd, 0x114514, 0, LWA_COLORKEY);//绿幕
			break;
		}
		case WM_Frame:
		{
			if(Target != (HWND)lParam)//当前窗口和之前的不符时
			{
				Target = (HWND)lParam;
			}
			InvalidateRect(hwnd, NULL, false);
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOREDRAW | SWP_NOACTIVATE);
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			RECT rc = { 0,0,g_ScreenSize.cx,g_ScreenSize.cy };
			
			//背景设为透明色
			FillRect(hdc, &rc, hBrBack);
			
			HGDIOBJ hOldPen = SelectObject(hdc, hPen);
			//空心画刷，不填充内部
			HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

			GetWindowRect(Target, &rc);
			Rectangle(hdc, rc.left + 2, rc.top + 2, rc.right - 2, rc.bottom - 2);

			// 恢复
			SelectObject(hdc, hOldBrush);
			SelectObject(hdc, hOldPen);
			
			EndPaint(hwnd, &ps);
			return 0;
		}
		case WM_DESTROY:
		{
			DeleteObject(hBrBack);
			DeleteObject(hPen);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//自动规则窗口过程
LRESULT CALLBACK AutoRuleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND ListHwnd;
	static HWND ButtonEnable;
	static HWND ButtonAdd;
	static HWND ButtonMod;
	static HWND ButtonDel;
	static INT SortIndex;

	static auto LoadList = []()->void
	{
		for(const auto &x : Index啥)
		{
			LVITEMW lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			lvi.lParam = x.first;
			lvi.pszText = const_cast<wchar_t *>(x.second->ClassName);
			ListView_InsertItem(ListHwnd, &lvi);

			ListView_SetItemText(ListHwnd, 0, 1, x.second->Path);
			ListView_SetItemText(ListHwnd, 0, 2, x.second->Flags & hasTITLE ? x.second->Title : L"");
			ListView_SetItemText(ListHwnd, 0, 3, x.second->Active ? L"启用" : L"禁用");
		}
	};

	constexpr int ID_Enable = 1;
	constexpr int ID_Add = 2;
	constexpr int ID_Mod = 3;
	constexpr int ID_Del = 4;


	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);
			//临时禁用事件钩子
			if(g_AutoRule)UnhookWinEvent(g_WindowHook);
			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(icex);
			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);
			ListHwnd = CreateWindowW(
				WC_LISTVIEW,
				NULL,
				WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
				g_FontSize.cx * 0,
				g_FontSize.cy * 0,
				g_FontSize.cx * 31,
				g_LineAnchorY[7],
				hwnd,
				NULL, NULL, NULL);
			ListView_SetExtendedListViewStyle(ListHwnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
			SendMessage(ListHwnd, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonEnable = CreateWindow(
				L"button",
				L"启禁用",
				WS_VISIBLE | WS_CHILD,
				32 * g_FontSize.cx,
				g_LineAnchorY[0],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_Enable,
				NULL, NULL
			);
			SendMessage(ButtonEnable, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonAdd = CreateWindow(
				L"button",
				L"添加",
				WS_VISIBLE | WS_CHILD,
				32 * g_FontSize.cx,
				g_LineAnchorY[1],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_Add,
				NULL, NULL
			);
			SendMessage(ButtonAdd, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonMod = CreateWindow(
				L"button",
				L"修改",
				WS_VISIBLE | WS_CHILD,
				32 * g_FontSize.cx,
				g_LineAnchorY[2],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_Mod,
				NULL, NULL
			);
			SendMessage(ButtonMod, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonDel = CreateWindow(
				L"button",
				L"删除",
				WS_VISIBLE | WS_CHILD,
				32 * g_FontSize.cx,
				g_LineAnchorY[3],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_Del,
				NULL, NULL
			);
			SendMessage(ButtonDel, WM_SETFONT, (WPARAM)g_EditFont, 0);


			LVCOLUMNW lvc;
			lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

			lvc.iSubItem = 0;
			lvc.pszText = L"窗口类名";
			lvc.cx = g_FontSize.cx * 5;
			ListView_InsertColumn(ListHwnd, 0, &lvc);

			lvc.iSubItem = 1;
			lvc.pszText = L"窗口所属进程路径";
			lvc.cx = g_FontSize.cx * 9 * 2;
			ListView_InsertColumn(ListHwnd, 1, &lvc);

			lvc.iSubItem = 2;
			lvc.pszText = L"窗口标题";
			lvc.cx = g_FontSize.cx * 5;
			ListView_InsertColumn(ListHwnd, 2, &lvc);

			lvc.iSubItem = 3;
			lvc.pszText = L"状态";
			lvc.cx = g_FontSize.cx * 3;
			ListView_InsertColumn(ListHwnd, 3, &lvc);

			LoadList();

			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case ID_Enable:
				{
					INT32 iFocus = ListView_GetNextItem(ListHwnd, -1, LVNI_FOCUSED);
					if(iFocus < 0)break;
					
					LVITEMW item{};
					item.iItem = iFocus;
					item.mask = LVIF_PARAM;
					ListView_GetItem(ListHwnd, &item);
					UINT16 index = item.lParam;

					V *v = Index啥[index];
					wchar_t buf[5];
					_snwprintf(buf, ARRAYSIZE(buf), L"%d", index);
					//修改操作
					if(v->Active)
					{
						v->Active = false;
						ListView_SetItemText(ListHwnd, iFocus, 3, L"禁用");
						WritePrivateProfileString(buf, L"Active", L"0", g_ConfigPath);
					}
					else
					{
						v->Active = true;
						ListView_SetItemText(ListHwnd, iFocus, 3, L"启用");
						WritePrivateProfileString(buf, L"Active", L"1", g_ConfigPath);
					}
					break;
				}
				case ID_Add:
				{
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 17 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[7] >> 1);
					rect.right = rect.left + 34 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[8];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_AutoRuleDetailHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - AutoRuleDetail",
						L"编辑明细",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_Mod:
				{
					INT32 iFocus = ListView_GetNextItem(ListHwnd, -1, LVNI_FOCUSED);
					if(iFocus < 0)break;

					LVITEMW item = {};
					item.iItem = iFocus;
					item.mask = LVIF_PARAM;
					ListView_GetItem(ListHwnd, &item);
					UINT16 index = item.lParam;

					V *v = Index啥[index];
					wchar_t buf[5];
					_snwprintf(buf, ARRAYSIZE(buf), L"%d", index);
					

					//修改操作
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 17 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[7] >> 1);
					rect.right = rect.left + 34 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[8];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					AutoRuleV *av = (AutoRuleV *)malloc(sizeof(AutoRuleV));
					av->index = index;
					av->target = v;
					
					g_AutoRuleDetailHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - AutoRuleDetail",
						L"编辑明细",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, av
					);
					break;
				}
				case ID_Del:
				{
					INT32 iFocus = ListView_GetNextItem(ListHwnd, -1, LVNI_FOCUSED);
					if(iFocus < 0)break;

					if(MessageBox(hwnd, L"确定要删除该项吗", L"提示", MB_YESNO | MB_ICONINFORMATION) == IDNO)break;
					
					LVITEMW item{};
					item.iItem = iFocus;
					item.mask = LVIF_PARAM;
					ListView_GetItem(ListHwnd, &item);
					UINT16 index = item.lParam;

					//先获取待删除目标信息
					V *v = Index啥[index];

					//删除哈希表映射
					Class啥.erase(v->ClassName);
					auto it = find(Class啥[v->ClassName].begin(), Class啥[v->ClassName].end(), v);
					if(it != Class啥[v->ClassName].end())
					{
						*it = move(Class啥[v->ClassName].back());
						Class啥[v->ClassName].pop_back(); // O(1)
					}
					if(Class啥[v->ClassName].empty())
					{
						Class啥.erase(v->ClassName);
					}
					Index啥.erase(index);
					
					//删除列表项
					wchar_t buf[5];
					_snwprintf(buf, ARRAYSIZE(buf), L"%d", index);
					ListView_DeleteItem(ListHwnd, iFocus);
					WritePrivateProfileSection(buf, L"", g_ConfigPath);
					DeleteIniSection(g_ConfigPath, buf);

					//最后才删除数据库数据，防止指针悬空
					*v = move(啥.back()); //将最后一个元素移动到v指向的位置
					啥.pop_back();        //删除最后一个元素
					break;
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			LPNMHDR phdr = (LPNMHDR)lParam;
			if(phdr->hwndFrom == ListHwnd)
			{
				if(phdr->code == LVN_COLUMNCLICK)
				{

					NMLISTVIEW *pnmv = (NMLISTVIEW *)lParam;
					int iColumn = pnmv->iSubItem;  // 哪一列被点了

					SortListAndUpdateArrow(ListHwnd, SortIndex, iColumn);
				}
				else if(phdr->code == NM_DBLCLK)
				{
					LPNMITEMACTIVATE pia = (LPNMITEMACTIVATE)lParam;
					int iRow = pia->iItem;     // 双击的行
					//int iCol = pia->iSubItem;  // 双击的列（如果点击到具体列）

					if(iRow < 0) break;
					

					LVITEMW item = {};
					item.iItem = iRow;
					item.mask = LVIF_PARAM;
					ListView_GetItem(ListHwnd, &item);
					UINT16 index = item.lParam;

					V *v = Index啥[index];
					wchar_t buf[5];
					_snwprintf(buf, ARRAYSIZE(buf), L"%d", index);


					//修改操作
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 17 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[7] >> 1);
					rect.right = rect.left + 34 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[8];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					AutoRuleV *av = (AutoRuleV *)malloc(sizeof(AutoRuleV));
					av->index = index;
					av->target = v;

					g_AutoRuleDetailHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - AutoRuleDetail",
						L"编辑明细",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, av
					);
					break;
				}
			}
			break;
		}
		case WM_Update:
		{
			if(wParam == 0)//新建模式
			{
				ListView_DeleteAllItems(ListHwnd);
				LoadList();
			}
			else
			{
				int iFoucs = ListView_GetNextItem(ListHwnd, -1, LVNI_FOCUSED);
				LVITEMW item = {};
				item.iItem = iFoucs;
				item.mask = LVIF_PARAM;
				ListView_GetItem(ListHwnd, &item);
				UINT16 index = item.lParam;

				ListView_SetItemText(ListHwnd, iFoucs, 0, Index啥[index]->ClassName);
				ListView_SetItemText(ListHwnd, iFoucs, 1, Index啥[index]->Path);
				ListView_SetItemText(ListHwnd, iFoucs, 2, Index啥[index]->Flags &hasTITLE ? Index啥[index]->Title : L"");
				ListView_SetItemText(ListHwnd, iFoucs, 3, Index啥[index]->Active ? L"启用" : L"禁用");
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(g_MainHwnd, true);
			//恢复事件钩子
			if(g_AutoRule)
			{
				g_WindowHook = HOOK
			}
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//自动规则详情窗口过程
LRESULT CALLBACK AutoRuleDetailProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditClassName;
	static HWND EditPath;
	static HWND EditTitle;
	static HWND ButtonActive;//Switch
	static HWND SButtonTransparent;//Switch
	static HWND EditTransparent;
	static HWND SButtonPos;//Switch
	static HWND EditX;
	static HWND EditY;
	static HWND SButtonSize;//Switch
	static HWND EditcX;
	static HWND EditcY;
	static HWND SButtonEnable;//Switch
	static HWND ButtonEnable;
	static HWND SButtonTopmost;//Switch
	static HWND ButtonTopmost;
	static HWND SButtonThrough;//Switch
	static HWND ButtonThrough;
	static HWND SButtonNewTitle;//Switch
	static HWND EditNewTitle;
	static HWND SButtonStyle;
	static HWND EditStyle;
	static HWND SButtonExStyle;
	static HWND EditExStyle;
	static HWND SButtonDelay;
	static HWND EditDelay;
	static HWND ButtonApply;
	static AutoRuleV *av;

	constexpr int ID_SActive = 1;
	constexpr int ID_STransparent = 2;
	constexpr int ID_SPos = 3;
	constexpr int ID_SSize = 4;
	constexpr int ID_SEnable = 5;
	constexpr int ID_Enable = 6;
	constexpr int ID_STopmost = 7;
	constexpr int ID_Topmost = 8;
	constexpr int ID_SThrough = 9;
	constexpr int ID_Through = 10;
	constexpr int ID_SNewTitle = 11;
	constexpr int ID_SStyle = 12;
	constexpr int ID_SExStyle = 13;
	constexpr int ID_SDelay = 14;
	constexpr int ID_Apply = 15;

	auto ButtonSizeProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_RBUTTONUP:
			{
				HMENU hPopupMenu = CreatePopupMenu();
				if(!hPopupMenu) break;

				// 根据 status 添加第一个项
				AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MAX, L"最大化");
				AppendMenu(hPopupMenu, MF_STRING, ID_MENU_NORMAL, L"正常");
				AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MIN, L"最小化");
				AppendMenu(hPopupMenu, MF_STRING, ID_MENU_HIDE, L"隐藏");
				CheckMenuItem(hPopupMenu, av->target->SizeOption, MF_BYCOMMAND | MF_CHECKED);
				// 获取鼠标位置（屏幕坐标）
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hwnd);
				// 显示菜单
				TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, GetParent(hwnd), NULL);

				DestroyMenu(hPopupMenu); // 别忘了销毁
				break;
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};


	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_AutoRuleHWND, false);
			EditClassName = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
				g_FontSize.cx * 5.5,
				g_LineAnchorY[0],
				g_FontSize.cx * 9.5,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditClassName, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonEnable = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[0],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SEnable,
				NULL,
				NULL
			);
			SendMessage(SButtonEnable, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonEnable = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_DISABLED,
				g_FontSize.cx * 25,
				g_LineAnchorY[0],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_Enable,
				NULL,
				NULL
			);
			SendMessage(ButtonEnable, WM_SETFONT, (WPARAM)g_EditFont, 0);


			EditPath = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
				g_FontSize.cx * 5.5,
				g_LineAnchorY[1],
				g_FontSize.cx * 9.5,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditPath, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonTopmost = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[1],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_STopmost,
				NULL,
				NULL
			);
			SendMessage(SButtonTopmost, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonTopmost = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_DISABLED,
				g_FontSize.cx * 25,
				g_LineAnchorY[1],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_Topmost,
				NULL,
				NULL
			);
			SendMessage(ButtonTopmost, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditTitle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
				g_FontSize.cx * 5.5,
				g_LineAnchorY[2],
				g_FontSize.cx * 9.5,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditTitle, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SendMessage(EditTitle, EM_SETCUEBANNER, TRUE, (LPARAM)L"可选");
			CreatePointToolTip(hwnd, EditTitle, L"留空即为不限", NULL, true, TTI_NONE, NULL, NULL);

			SButtonThrough = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[2],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SThrough,
				NULL,
				NULL
			);
			SendMessage(SButtonThrough, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonThrough = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_DISABLED,
				g_FontSize.cx * 25,
				g_LineAnchorY[2],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_Through,
				NULL,
				NULL
			);
			SendMessage(ButtonThrough, WM_SETFONT, (WPARAM)g_EditFont, 0);


			ButtonActive = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 6,
				g_LineAnchorY[3],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SActive,
				NULL,
				NULL
			);
			SendMessage(ButtonActive, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonNewTitle = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[3],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SNewTitle,
				NULL,
				NULL
			);
			SendMessage(SButtonNewTitle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditNewTitle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_DISABLED,
				g_FontSize.cx * 24,
				g_LineAnchorY[3],
				g_FontSize.cx * 8,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditNewTitle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonTransparent = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 6,
				g_LineAnchorY[4],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_STransparent,
				NULL,
				NULL
			);
			SendMessage(SButtonTransparent, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditTransparent = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_DISABLED,
				g_FontSize.cx * 8,
				g_LineAnchorY[4],
				g_FontSize.cx * 5,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditTransparent, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonStyle = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[4],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SStyle,
				NULL,
				NULL
			);
			SendMessage(SButtonStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditStyle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED,
				g_FontSize.cx * 24,
				g_LineAnchorY[4],
				g_FontSize.cx * 8,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonPos = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 6,
				g_LineAnchorY[5],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SPos,
				NULL,
				NULL
			);
			SendMessage(SButtonPos, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditX = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_DISABLED,
				g_FontSize.cx * 8,
				g_LineAnchorY[5],
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditX, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditY = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_DISABLED,
				g_FontSize.cx * 12,
				g_LineAnchorY[5],
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditY, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonExStyle = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[5],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SExStyle,
				NULL,
				NULL
			);
			SendMessage(SButtonExStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditExStyle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | WS_DISABLED,
				g_FontSize.cx * 24,
				g_LineAnchorY[5],
				g_FontSize.cx * 8,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditExStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonSize = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 6,
				g_LineAnchorY[6],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SSize,
				NULL,
				NULL
			);
			SendMessage(SButtonSize, WM_SETFONT, (WPARAM)g_EditFont, 0);
			CreatePointToolTip(hwnd, SButtonSize, L"右键显示更多选项", NULL, true, TTI_NONE, NULL, NULL);
			SetWindowSubclass(SButtonSize, ButtonSizeProc, 1, 0);

			EditcX = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_DISABLED,
				g_FontSize.cx * 8,
				g_LineAnchorY[6],
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditcX, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditcY = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | ES_AUTOHSCROLL | WS_DISABLED,
				g_FontSize.cx * 12,
				g_LineAnchorY[6],
				g_FontSize.cx * 3,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditcY, WM_SETFONT, (WPARAM)g_EditFont, 0);

			SButtonDelay = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 22,
				g_LineAnchorY[6],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_SDelay,
				NULL,
				NULL
			);
			SendMessage(SButtonDelay, WM_SETFONT, (WPARAM)g_EditFont, 0);

			EditDelay = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER | WS_DISABLED,
				g_FontSize.cx * 24,
				g_LineAnchorY[6],
				g_FontSize.cx * 8,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditDelay, WM_SETFONT, (WPARAM)g_EditFont, 0);

			ButtonApply = CreateWindow(
				L"button",
				L"确定",
				WS_VISIBLE | WS_CHILD,
				29 * g_FontSize.cx,
				g_LineAnchorY[7],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_Apply,
				NULL, NULL
			);
			SendMessage(ButtonApply, WM_SETFONT, (WPARAM)g_EditFont, 0);

			av = (AutoRuleV *)((CREATESTRUCT *)lParam)->lpCreateParams;
			
			if(av == NULL)//新建模式
			{
				av = (AutoRuleV *)malloc(sizeof(AutoRuleV));
				av->index = 0;
				av->target = (V *)calloc(1, sizeof(V));
				av->target->SizeOption = ID_MENU_NORMAL;
				if(IsWindow(g_EditHandle))//主窗口中的信息有效
				{
					wchar_t temp[MAX_PATH];
					GetWindowText(g_StaticExename, temp, ARRAYSIZE(temp));
					SetWindowText(EditPath, temp);
					GetWindowText(g_StaticClassname, temp, ARRAYSIZE(temp));
					SetWindowText(EditClassName, temp);
					GetWindowText(g_EditTitlename, temp, ARRAYSIZE(temp));
					SetWindowText(EditTitle, temp);
				}
			}
			else//编辑模式
			{
				V *v = av->target;
				wchar_t temp[21];

				SetWindowText(EditClassName, av->target->ClassName);

				SetWindowText(EditPath, v->Path);

				if(v->Flags & hasTITLE)SetWindowText(EditTitle, v->Title);

				if(v->Active)SendMessage(ButtonActive, BM_SETCHECK, BST_CHECKED, 0);

				if(v->Flags & WH_TRANSPARENT)
				{
					SendMessage(SButtonTransparent, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditTransparent, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->Trans);
				SetWindowText(EditTransparent, temp);

				if(v->Flags & WH_POS)
				{
					SendMessage(SButtonPos, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditX, true);
					EnableWindow(EditY, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->X);
				SetWindowText(EditX, temp);
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->Y);
				SetWindowText(EditY, temp);

				if(v->Flags & WH_SIZE)
				{
					SendMessage(SButtonSize, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditcX, true);
					EnableWindow(EditcY, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->cX);
				SetWindowText(EditcX, temp);
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->cY);
				SetWindowText(EditcY, temp);

				if(v->Flags & WH_ENABLE)
				{
					SendMessage(SButtonEnable, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(ButtonEnable, true);
				}
				if(v->Enable)SendMessage(ButtonEnable, BM_SETCHECK, BST_CHECKED, 0);

				if(v->Flags & WH_TOPMOST)
				{
					SendMessage(SButtonTopmost, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(ButtonTopmost, true);
				}
				if(v->Topmost)SendMessage(ButtonTopmost, BM_SETCHECK, BST_CHECKED, 0);

				if(v->Flags & WH_THROUGH)
				{
					SendMessage(SButtonThrough, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(ButtonThrough, true);
				}
				if(v->Through)SendMessage(ButtonThrough, BM_SETCHECK, BST_CHECKED, 0);

				if(v->Flags & WH_TITLE)
				{
					SendMessage(SButtonNewTitle, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditNewTitle, true);
				}
				SetWindowText(EditNewTitle, v->NewTitle);

				if(v->Flags & WH_STYLE)
				{
					SendMessage(SButtonStyle, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditStyle, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"0x%08x", v->Style);
				SetWindowText(EditStyle, temp);

				if(v->Flags & WH_EXSTYLE)
				{
					SendMessage(SButtonExStyle, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditExStyle, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"0x%08x", v->ExStyle);
				SetWindowText(EditExStyle, temp);

				if(v->Flags & WH_DELAY)
				{
					SendMessage(SButtonDelay, BM_SETCHECK, BST_CHECKED, 0);
					EnableWindow(EditDelay, true);
				}
				_snwprintf(temp, ARRAYSIZE(temp), L"%d", v->Delay);
				SetWindowText(EditDelay, temp);
			}

			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			HGDIOBJ OldObj = SelectObject(hdc, g_Font);
			RECT RectDrawText;
			RectDrawText.left = 1 * g_FontSize.cx;
			RectDrawText.right = 5 * g_FontSize.cx;
			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口类名"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("进程路径"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[2];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口标题"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[3];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("激活规则"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[4];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("不透明度"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[5];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("位置修改"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[6];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("大小修改"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);


			RectDrawText.left = 17 * g_FontSize.cx;
			RectDrawText.right = 22 * g_FontSize.cx;
			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;

			DrawText(hdc, TEXT("启禁窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("设置置顶"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[2];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("设置穿透"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[3];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("修改标题"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[4];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("普通风格"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[5];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("扩展风格"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[6];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("设置延迟"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			SelectObject(hdc, OldObj);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_COMMAND:
		{
			V *v = av->target;
			if(HIWORD(wParam) != BN_CLICKED)break;//仅处理按钮点击事件
			switch(LOWORD(wParam))
			{
				case ID_SActive:
				{
					v->Active = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
					break;
				}
				case ID_STransparent:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_TRANSPARENT;
						EnableWindow(EditTransparent, true);
					}
					else
					{
						v->Flags &= ~WH_TRANSPARENT;
						EnableWindow(EditTransparent, false);
					}
					break;
				}
				case ID_SPos:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_POS;
						EnableWindow(EditX, true);
						EnableWindow(EditY, true);
					}
					else
					{
						v->Flags &= ~WH_POS;
						EnableWindow(EditX, false);
						EnableWindow(EditY, false);
					}
					break;
				}
				case ID_SSize:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_SIZE;
						EnableWindow(EditcX, true);
						EnableWindow(EditcY, true);
					}
					else
					{
						v->Flags &= ~WH_SIZE;
						EnableWindow(EditcX, false);
						EnableWindow(EditcY, false);
					}
					break;
				}
				case ID_SEnable:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_ENABLE;
						EnableWindow(ButtonEnable, true);
					}
					else
					{
						v->Flags &= ~WH_ENABLE;
						EnableWindow(ButtonEnable, false);
					}
					break;
				}
				case ID_STopmost:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_TOPMOST;
						EnableWindow(ButtonTopmost, true);
					}
					else
					{
						v->Flags &= ~WH_TOPMOST;
						EnableWindow(ButtonTopmost, false);
					}
					break;
				}
				case ID_SThrough:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_THROUGH;
						EnableWindow(ButtonThrough, true);
					}
					else
					{
						v->Flags &= ~WH_THROUGH;
						EnableWindow(ButtonThrough, false);
					}
					break;
				}
				case ID_SNewTitle:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_TITLE;
						EnableWindow(EditNewTitle, true);
					}
					else
					{
						v->Flags &= ~WH_TITLE;
						EnableWindow(EditNewTitle, false);
					}
					break;
				}
				case ID_SStyle:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_STYLE;
						EnableWindow(EditStyle, true);
					}
					else
					{
						v->Flags &= ~WH_STYLE;
						EnableWindow(EditStyle, false);
					}
					break;
				}
				case ID_SExStyle:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_EXSTYLE;
						EnableWindow(EditExStyle, true);
					}
					else
					{
						v->Flags &= ~WH_EXSTYLE;
						EnableWindow(EditExStyle, false);
					}
					break;
				}
				case ID_SDelay:
				{
					if(SendMessage((HWND)lParam, BM_GETCHECK, 0, 0))
					{
						v->Flags |= WH_DELAY;
						EnableWindow(EditDelay, true);
					}
					else
					{
						v->Flags &= ~WH_DELAY;
						EnableWindow(EditDelay, false);
					}
					break;
				}
				case ID_Apply:
				{
					wchar_t temp[MAX_PATH];
					GetWindowText(EditClassName, v->ClassName, ARRAYSIZE(v->ClassName));
					if(v->ClassName[0] == L'\0')
					{
						RECT rect;
						GetWindowRect(EditClassName, &rect);
						ShowTooltip(EditClassName, NULL, { (rect.right + rect.left) >> 1,rect.bottom }, L"窗口类名不能为空", NULL, true, false, TTI_NONE, 3000);
						break;
					}

					GetWindowText(EditPath, v->Path, ARRAYSIZE(v->Path));
					if(v->Path[0] == L'\0')
					{
						RECT rect;
						GetWindowRect(EditPath, &rect);
						ShowTooltip(EditPath, NULL, { (rect.right + rect.left) >> 1,rect.bottom }, L"进程路径不能为空", NULL, true, false, TTI_NONE, 3000);
						break;
					}


					GetWindowText(EditTitle, v->Title, ARRAYSIZE(v->Title));
					if(v->Title[0] == L'\0')
					{
						v->Flags &= ~hasTITLE;
					}
					else
					{
						v->Flags |= hasTITLE;
					}

					GetWindowText(EditTransparent, temp, ARRAYSIZE(temp));
					v->Trans = wcstol(temp, NULL, 10);
					if(v->Trans < 0 || v->Trans > 255)
					{
						v->Flags &= ~WH_TRANSPARENT;
					}

					GetWindowText(EditX, temp, ARRAYSIZE(temp));
					v->X = wcstol(temp, NULL, 10);
					GetWindowText(EditY, temp, ARRAYSIZE(temp));
					v->Y = wcstol(temp, NULL, 10);

					GetWindowText(EditcX, temp, ARRAYSIZE(temp));
					v->cX = wcstol(temp, NULL, 10);
					GetWindowText(EditcY, temp, ARRAYSIZE(temp));
					v->cY = wcstol(temp, NULL, 10);

					v->Enable = SendMessage(ButtonEnable, BM_GETCHECK, 0, 0);

					v->Topmost = SendMessage(ButtonTopmost, BM_GETCHECK, 0, 0);

					v->Through = SendMessage(ButtonThrough, BM_GETCHECK, 0, 0);

					GetWindowText(EditNewTitle, v->NewTitle, ARRAYSIZE(v->NewTitle));

					GetWindowText(EditStyle, temp, ARRAYSIZE(temp));
					v->Style = wcstol(temp, NULL, 16);

					GetWindowText(EditExStyle, temp, ARRAYSIZE(temp));
					v->ExStyle = wcstol(temp, NULL, 16);

					GetWindowText(EditDelay, temp, ARRAYSIZE(temp));
					v->Delay = wcstol(temp, NULL, 10);

					GetWindowText(EditClassName, temp, ARRAYSIZE(temp));
					SaveRules(v, av->index);
					if(av->index == 0)//新建模式
					{
						SendMessage(g_AutoRuleHWND, WM_Update, 0, 0);
					}
					else//编辑模式
					{
						SendMessage(g_AutoRuleHWND, WM_Update, 1, 0);
					}
					DestroyWindow(hwnd);
					break;
				}
				case ID_MENU_MAX:
				case ID_MENU_NORMAL:
				case ID_MENU_MIN:
				case ID_MENU_HIDE:
				{
					v->SizeOption = LOWORD(wParam);
					break;
				}
			}
			break;
		}
		case WM_DESTROY:
		{
			EnableWindow(g_AutoRuleHWND, true);
			if(av->index == 0)//新建模式
			{
				free(av->target);
			}
			free(av);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//快捷键注册窗口过程
LRESULT CALLBACK HotkeyProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditHotkeyShowMainwindow;
	static HWND EditHotkeySelect;
	static HWND EditHotkeySelectParent;
	static HWND EditHotkeyTransparent;
	static HWND EditHotkeyTopmost;
	static HWND EditHotkeyMove;
	static HWND EditHotkeySize;
	static HWND ButtonApply;

	static auto MainwindowProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是修饰键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					wchar_t KeyName[64] = { 0 };
					LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
					switch(wParam)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;

					SetWindowTextW(hwnd, KeyCombo.c_str());
					SetProp(hwnd, L"Hotkey", 
							(HANDLE)(UINT_PTR)
							(
								(wParam << 6) |
								(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0))<<3)|
								((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
							)
					);
				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto SelectProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是修饰键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					wchar_t KeyName[64] = { 0 };
					LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
					switch(wParam)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;

					SetWindowTextW(hwnd, KeyCombo.c_str());
					SetProp(hwnd, L"Hotkey",
							(HANDLE)(UINT_PTR)
							(
								(wParam << 6) |
								(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
								((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
								)
					);
				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto SelectParentProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是修饰键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					wchar_t KeyName[64] = { 0 };
					LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
					switch(wParam)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;

					SetWindowTextW(hwnd, KeyCombo.c_str());
					SetProp(hwnd, L"Hotkey",
							(HANDLE)(UINT_PTR)
							(
								(wParam << 6) |
								(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
								((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
								)
					);
				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto TransparentProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是Win键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					if(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU)//只有修饰键
					{
						KeyCombo += L"滚轮";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)(
							(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
							((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
							)
						);
					}
					else//普通情况
					{
						wchar_t KeyName[64] = { 0 };
						LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
						switch(wParam)
						{
							// 主键区方向键与导航键才需要加扩展标志
							case VK_INSERT: case VK_DELETE:
							case VK_HOME: case VK_END:
							case VK_PRIOR: case VK_NEXT:
							case VK_LEFT: case VK_RIGHT:
							case VK_UP: case VK_DOWN:
							case VK_DIVIDE: // 小键盘 / 也是扩展键
							case VK_RCONTROL: case VK_RMENU:
								lParam |= (1 << 24);
								break;
						}
						GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
						KeyCombo += KeyName;
						KeyCombo += L" + 滚轮";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey",
								(HANDLE)(UINT_PTR)
								(
									(wParam << 6) |
									(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
									((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
									)
						);
					}
					
				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto TopmostProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是修饰键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU || wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					wchar_t KeyName[64] = { 0 };
					LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
					switch(wParam)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;

					SetWindowTextW(hwnd, KeyCombo.c_str());
					SetProp(hwnd, L"Hotkey",
							(HANDLE)(UINT_PTR)
							(
								(wParam << 6) |
								(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
								((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
								)
					);
				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto MoveProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是Win键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					if(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU)//只有修饰键
					{
						KeyCombo += L"左键";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)(
							(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
							((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
							)
						);
					}
					else//普通情况
					{
						wchar_t KeyName[64] = { 0 };
						LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
						switch(wParam)
						{
							// 主键区方向键与导航键才需要加扩展标志
							case VK_INSERT: case VK_DELETE:
							case VK_HOME: case VK_END:
							case VK_PRIOR: case VK_NEXT:
							case VK_LEFT: case VK_RIGHT:
							case VK_UP: case VK_DOWN:
							case VK_DIVIDE: // 小键盘 / 也是扩展键
							case VK_RCONTROL: case VK_RMENU:
								lParam |= (1 << 24);
								break;
						}
						GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
						KeyCombo += KeyName;
						KeyCombo += L" + 左键";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey",
								(HANDLE)(UINT_PTR)
								(
									(wParam << 6) |
									(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
									((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
									)
						);
					}

				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	static auto SizeProc = [](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)->LRESULT
	{
		switch(msg)
		{
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			{
				//按 Delete 或 Backspace 清空
				if(wParam == VK_BACK || wParam == VK_DELETE)
				{
					SetWindowTextW(hwnd, L"");
					SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)NULL);
					return 0;
				}

				//检测是否是组合键
				bool Lctrl = GetAsyncKeyState(VK_LCONTROL) & 0x8000;
				bool Lshift = GetAsyncKeyState(VK_LSHIFT) & 0x8000;
				bool Lalt = GetAsyncKeyState(VK_LMENU) & 0x8000;

				bool Rctrl = GetAsyncKeyState(VK_RCONTROL) & 0x8000;
				bool Rshift = GetAsyncKeyState(VK_RSHIFT) & 0x8000;
				bool Ralt = GetAsyncKeyState(VK_RMENU) & 0x8000;

				//只有当有修饰键按下且当前键不是Win键时才响应
				if((Lctrl || Lshift || Lalt || Rctrl || Rshift || Ralt) && !(wParam == VK_LWIN || wParam == VK_RWIN))
				{
					wstring KeyCombo;
					//Ctrl
					if(Lctrl && Rctrl)
					{
						KeyCombo += L"Ctrl + ";
					}
					else if(Lctrl)
					{
						KeyCombo += L"LCtrl + ";
					}
					else if(Rctrl)
					{
						KeyCombo += L"RCtrl + ";
					}
					//Shift
					if(Lshift && Rshift)
					{
						KeyCombo += L"Shift + ";
					}
					else if(Lshift)
					{
						KeyCombo += L"LShift + ";
					}
					else if(Rshift)
					{
						KeyCombo += L"RShift + ";
					}
					//Alt
					if(Lalt && Ralt)
					{
						KeyCombo += L"Alt + ";
					}
					else if(Lalt)
					{
						KeyCombo += L"LAlt + ";
					}
					else if(Ralt)
					{
						KeyCombo += L"RAlt + ";
					}

					if(wParam == VK_CONTROL || wParam == VK_SHIFT || wParam == VK_MENU)//只有修饰键
					{
						KeyCombo += L"右键";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey", (HANDLE)(UINT_PTR)(
							(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
							((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
							)
						);
					}
					else//普通情况
					{
						wchar_t KeyName[64] = { 0 };
						LONG lParam = (MapVirtualKey(wParam, MAPVK_VK_TO_VSC) << 16);
						switch(wParam)
						{
							// 主键区方向键与导航键才需要加扩展标志
							case VK_INSERT: case VK_DELETE:
							case VK_HOME: case VK_END:
							case VK_PRIOR: case VK_NEXT:
							case VK_LEFT: case VK_RIGHT:
							case VK_UP: case VK_DOWN:
							case VK_DIVIDE: // 小键盘 / 也是扩展键
							case VK_RCONTROL: case VK_RMENU:
								lParam |= (1 << 24);
								break;
						}
						GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
						KeyCombo += KeyName;
						KeyCombo += L" + 右键";
						SetWindowTextW(hwnd, KeyCombo.c_str());
						SetProp(hwnd, L"Hotkey",
								(HANDLE)(UINT_PTR)
								(
									(wParam << 6) |
									(((Lctrl ? MOD_CONTROL : 0) | (Lshift ? MOD_SHIFT : 0) | (Lalt ? MOD_ALT : 0)) << 3) |
									((Rctrl ? MOD_CONTROL : 0) | (Rshift ? MOD_SHIFT : 0) | (Ralt ? MOD_ALT : 0))
									)
						);
					}

				}

				return 0;//阻止默认输入
			}
		}
		return DefSubclassProc(hwnd, msg, wParam, lParam);
	};

	switch(msg)
	{
		case WM_CREATE:
		{
			EnableWindow(g_MainHwnd, false);

			//临时注销
			UnregisterHotKey(g_MainHwnd, ID_Hotkey_Mainwindow);
			UnregisterHotKey(g_MainHwnd, ID_Hotkey_Select);
			UnregisterHotKey(g_MainHwnd, ID_Hotkey_SelectParent);
			if(g_MouseHook)UnhookWindowsHookEx(g_MouseHook);//Transparent、Move、Size的快捷键
			UnregisterHotKey(g_MainHwnd, ID_Hotkey_Topmost);

			EditHotkeyShowMainwindow = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[1],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeyShowMainwindow, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeyShowMainwindow, MainwindowProc, 1, 0);
			if(g_HotkeyShowMainwindow)
			{
				wchar_t KeyName[64];
				LONG lParam = (MapVirtualKey(g_HotkeyShowMainwindow >> 6, MAPVK_VK_TO_VSC) << 16);
				switch(g_HotkeyShowMainwindow >> 6)
				{
					// 主键区方向键与导航键才需要加扩展标志
					case VK_INSERT: case VK_DELETE:
					case VK_HOME: case VK_END:
					case VK_PRIOR: case VK_NEXT:
					case VK_LEFT: case VK_RIGHT:
					case VK_UP: case VK_DOWN:
					case VK_DIVIDE: // 小键盘 / 也是扩展键
					case VK_RCONTROL: case VK_RMENU:
						lParam |= (1 << 24);
						break;
				}
				GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
				wstring KeyCombo;
				bool Lctrl = g_HotkeyShowMainwindow & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeyShowMainwindow & MOD_CONTROL;
				bool Lshift = g_HotkeyShowMainwindow & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeyShowMainwindow & MOD_SHIFT;
				bool Lalt = g_HotkeyShowMainwindow & (MOD_ALT << 3);
				bool Ralt = g_HotkeyShowMainwindow & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				KeyCombo += KeyName;
				SetWindowText(EditHotkeyShowMainwindow, KeyCombo.c_str());
				SetProp(EditHotkeyShowMainwindow, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeyShowMainwindow);
			}
			
			
			EditHotkeySelect = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[2],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeySelect, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeySelect, SelectProc, 1, 0);
			if(g_HotkeySelect)
			{
				wchar_t KeyName[64];
				LONG lParam = (MapVirtualKey(g_HotkeySelect >> 6, MAPVK_VK_TO_VSC) << 16);
				switch(g_HotkeySelect >> 6)
				{
					// 主键区方向键与导航键才需要加扩展标志
					case VK_INSERT: case VK_DELETE:
					case VK_HOME: case VK_END:
					case VK_PRIOR: case VK_NEXT:
					case VK_LEFT: case VK_RIGHT:
					case VK_UP: case VK_DOWN:
					case VK_DIVIDE: // 小键盘 / 也是扩展键
					case VK_RCONTROL: case VK_RMENU:
						lParam |= (1 << 24);
						break;
				}
				GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
				wstring KeyCombo;
				bool Lctrl = g_HotkeySelect & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeySelect & MOD_CONTROL;
				bool Lshift = g_HotkeySelect & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeySelect & MOD_SHIFT;
				bool Lalt = g_HotkeySelect & (MOD_ALT << 3);
				bool Ralt = g_HotkeySelect & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				KeyCombo += KeyName;
				SetWindowText(EditHotkeySelect, KeyCombo.c_str());
				SetProp(EditHotkeySelect, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeySelect);
			}


			EditHotkeySelectParent = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[3],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeySelectParent, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeySelectParent, SelectParentProc, 1, 0);
			if(g_HotkeySelectParent)
			{
				wchar_t KeyName[64];
				LONG lParam = (MapVirtualKey(g_HotkeySelectParent >> 6, MAPVK_VK_TO_VSC) << 16);
				switch(g_HotkeySelectParent >> 6)
				{
					// 主键区方向键与导航键才需要加扩展标志
					case VK_INSERT: case VK_DELETE:
					case VK_HOME: case VK_END:
					case VK_PRIOR: case VK_NEXT:
					case VK_LEFT: case VK_RIGHT:
					case VK_UP: case VK_DOWN:
					case VK_DIVIDE: // 小键盘 / 也是扩展键
					case VK_RCONTROL: case VK_RMENU:
						lParam |= (1 << 24);
						break;
				}
				GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
				wstring KeyCombo;
				bool Lctrl = g_HotkeySelectParent & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeySelectParent & MOD_CONTROL;
				bool Lshift = g_HotkeySelectParent & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeySelectParent & MOD_SHIFT;
				bool Lalt = g_HotkeySelectParent & (MOD_ALT << 3);
				bool Ralt = g_HotkeySelectParent & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				KeyCombo += KeyName;
				SetWindowText(EditHotkeySelectParent, KeyCombo.c_str());
				SetProp(EditHotkeySelectParent, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeySelectParent);
			}
			

			EditHotkeyTransparent = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[4],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeyTransparent, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeyTransparent, TransparentProc, 1, 0);
			if(g_HotkeyTransparent)
			{
				wstring KeyCombo;
				bool Lctrl = g_HotkeyTransparent & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeyTransparent & MOD_CONTROL;
				bool Lshift = g_HotkeyTransparent & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeyTransparent & MOD_SHIFT;
				bool Lalt = g_HotkeyTransparent & (MOD_ALT << 3);
				bool Ralt = g_HotkeyTransparent & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				wchar_t KeyName[64] = { 0 };
				if(g_HotkeyTransparent >> 6)
				{
					LONG lParam = (MapVirtualKey(g_HotkeyTransparent >> 6, MAPVK_VK_TO_VSC) << 16);
					switch(g_HotkeyTransparent >> 6)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;
					KeyCombo += L" + 滚轮";
				}
				else
				{
					KeyCombo += L"滚轮";
				}
				SetWindowText(EditHotkeyTransparent, KeyCombo.c_str());
				SetProp(EditHotkeyTransparent, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeyTransparent);
			}
			

			EditHotkeyTopmost = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[5],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeyTopmost, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeyTopmost, TopmostProc, 1, 0);
			if(g_HotkeyTopmost)
			{
				wchar_t KeyName[64];
				LONG lParam = (MapVirtualKey(g_HotkeyTopmost >> 6, MAPVK_VK_TO_VSC) << 16);
				switch(g_HotkeyTopmost >> 6)
				{
					// 主键区方向键与导航键才需要加扩展标志
					case VK_INSERT: case VK_DELETE:
					case VK_HOME: case VK_END:
					case VK_PRIOR: case VK_NEXT:
					case VK_LEFT: case VK_RIGHT:
					case VK_UP: case VK_DOWN:
					case VK_DIVIDE: // 小键盘 / 也是扩展键
					case VK_RCONTROL: case VK_RMENU:
						lParam |= (1 << 24);
						break;
				}
				GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
				wstring KeyCombo;
				bool Lctrl = g_HotkeyTopmost & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeyTopmost & MOD_CONTROL;
				bool Lshift = g_HotkeyTopmost & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeyTopmost & MOD_SHIFT;
				bool Lalt = g_HotkeyTopmost & (MOD_ALT << 3);
				bool Ralt = g_HotkeyTopmost & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				KeyCombo += KeyName;
				SetWindowText(EditHotkeyTopmost, KeyCombo.c_str());
				SetProp(EditHotkeyTopmost, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeyTopmost);
			}


			EditHotkeyMove = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[6],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeyMove, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeyMove, MoveProc, 1, 0);
			if(g_HotkeyMove)
			{
				wstring KeyCombo;
				bool Lctrl = g_HotkeyMove & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeyMove & MOD_CONTROL;
				bool Lshift = g_HotkeyMove & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeyMove & MOD_SHIFT;
				bool Lalt = g_HotkeyMove & (MOD_ALT << 3);
				bool Ralt = g_HotkeyMove & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				wchar_t KeyName[64] = { 0 };
				if(g_HotkeyMove >> 6)
				{
					LONG lParam = (MapVirtualKey(g_HotkeyMove >> 6, MAPVK_VK_TO_VSC) << 16);
					switch(g_HotkeyMove >> 6)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;
					KeyCombo += L" + 左键";
				}
				else
				{
					KeyCombo += L"左键";
				}
				SetWindowText(EditHotkeyMove, KeyCombo.c_str());
				SetProp(EditHotkeyMove, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeyMove);
			}


			EditHotkeySize = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_READONLY | ES_CENTER | ES_AUTOHSCROLL,
				9 * g_FontSize.cx,
				g_LineAnchorY[7],
				g_FontSize.cx * 14,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(EditHotkeySize, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(EditHotkeySize, SizeProc, 1, 0);
			if(g_HotkeySize)
			{
				wstring KeyCombo;
				bool Lctrl = g_HotkeySize & (MOD_CONTROL << 3);
				bool Rctrl = g_HotkeySize & MOD_CONTROL;
				bool Lshift = g_HotkeySize & (MOD_SHIFT << 3);
				bool Rshift = g_HotkeySize & MOD_SHIFT;
				bool Lalt = g_HotkeySize & (MOD_ALT << 3);
				bool Ralt = g_HotkeySize & MOD_ALT;
				//Ctrl
				if(Lctrl && Rctrl)
				{
					KeyCombo += L"Ctrl + ";
				}
				else if(Lctrl)
				{
					KeyCombo += L"LCtrl + ";
				}
				else if(Rctrl)
				{
					KeyCombo += L"RCtrl + ";
				}
				//Shift
				if(Lshift && Rshift)
				{
					KeyCombo += L"Shift + ";
				}
				else if(Lshift)
				{
					KeyCombo += L"LShift + ";
				}
				else if(Rshift)
				{
					KeyCombo += L"RShift + ";
				}
				//Alt
				if(Lalt && Ralt)
				{
					KeyCombo += L"Alt + ";
				}
				else if(Lalt)
				{
					KeyCombo += L"LAlt + ";
				}
				else if(Ralt)
				{
					KeyCombo += L"RAlt + ";
				}
				wchar_t KeyName[64] = { 0 };
				if(g_HotkeySize >> 6)
				{
					LONG lParam = (MapVirtualKey(g_HotkeySize >> 6, MAPVK_VK_TO_VSC) << 16);
					switch(g_HotkeySize >> 6)
					{
						// 主键区方向键与导航键才需要加扩展标志
						case VK_INSERT: case VK_DELETE:
						case VK_HOME: case VK_END:
						case VK_PRIOR: case VK_NEXT:
						case VK_LEFT: case VK_RIGHT:
						case VK_UP: case VK_DOWN:
						case VK_DIVIDE: // 小键盘 / 也是扩展键
						case VK_RCONTROL: case VK_RMENU:
							lParam |= (1 << 24);
							break;
					}
					GetKeyNameText(lParam, KeyName, ARRAYSIZE(KeyName));
					KeyCombo += KeyName;
					KeyCombo += L" + 右键";
				}
				else
				{
					KeyCombo += L"右键";
				}
				SetWindowText(EditHotkeySize, KeyCombo.c_str());
				SetProp(EditHotkeySize, L"Hotkey", (HANDLE)(UINT_PTR)g_HotkeySize);
			}


			ButtonApply = CreateWindow(
				L"button",
				L"确定",
				WS_VISIBLE | WS_CHILD,
				22 * g_FontSize.cx,
				g_LineAnchorY[8],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)114,
				NULL, NULL
			);
			SendMessage(ButtonApply, WM_SETFONT, (WPARAM)g_EditFont, 0);


			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			SetBkMode(hdc, TRANSPARENT);
			HGDIOBJ OldObj = SelectObject(hdc, g_Font);

			RECT RectDrawText;
			RectDrawText.left = 1.5 * g_FontSize.cx;
			RectDrawText.right = 24 * g_FontSize.cx;

			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("Ctrl、Shift、Alt区分左右，同时按住以取消区分"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("显示主窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[2];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("选择子窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[3];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("选择根窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[4];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("不透明调整"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[5];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("切置顶状态"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);
			
			RectDrawText.top = g_LineAnchorY[6];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("拖动某窗口"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[7];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("改窗口大小"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);

			SelectObject(hdc, OldObj);

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam) == 114)
			{
				wchar_t buf[64];
				//ShowMain的快捷键
				g_HotkeyShowMainwindow = (UINT)(UINT_PTR)GetProp(EditHotkeyShowMainwindow, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeyShowMainwindow);
				WritePrivateProfileString(L"Setting", L"ShowMainwindowHotkey", buf, g_ConfigPath);

				//Select的快捷键
				g_HotkeySelect = (UINT)(UINT_PTR)GetProp(EditHotkeySelect, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeySelect);
				WritePrivateProfileString(L"Setting", L"SelectHotkey", buf, g_ConfigPath);

				//SelectParent的快捷键
				g_HotkeySelectParent = (UINT)(UINT_PTR)GetProp(EditHotkeySelectParent, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeySelectParent);
				WritePrivateProfileString(L"Setting", L"SelectParentHotkey", buf, g_ConfigPath);

				//Transparent的快捷键
				g_HotkeyTransparent = (UINT)(UINT_PTR)GetProp(EditHotkeyTransparent, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeyTransparent);
				WritePrivateProfileString(L"Setting", L"TransparentHotkey", buf, g_ConfigPath);

				//Topmost的快捷键
				g_HotkeyTopmost = (UINT)(UINT_PTR)GetProp(EditHotkeyTopmost, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeyTopmost);
				WritePrivateProfileString(L"Setting", L"TopmostHotkey", buf, g_ConfigPath);

				//Move的快捷键
				g_HotkeyMove = (UINT)(UINT_PTR)GetProp(EditHotkeyMove, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeyMove);
				WritePrivateProfileString(L"Setting", L"MoveHotkey", buf, g_ConfigPath);

				//Size的快捷键
				g_HotkeySize = (UINT)(UINT_PTR)GetProp(EditHotkeySize, L"Hotkey");
				_snwprintf(buf, ARRAYSIZE(buf), L"%u", g_HotkeySize);
				WritePrivateProfileString(L"Setting", L"SizeHotkey", buf, g_ConfigPath);

				DestroyWindow(hwnd);
			}
			break;
		}
		case WM_DESTROY:
		{
			//ShowMain的快捷键
			
			if(g_HotkeyShowMainwindow && !RegisterHotKey(g_MainHwnd, ID_Hotkey_Mainwindow, (g_HotkeyShowMainwindow & 0b111) | ((g_HotkeyShowMainwindow >> 3) & 0b111), g_HotkeyShowMainwindow >> 6))
			{
				MessageBox(hwnd, L"显示主窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			//Select的快捷键
			if(g_HotkeySelect && !RegisterHotKey(g_MainHwnd, ID_Hotkey_Select, (g_HotkeySelect & 0b111) | ((g_HotkeySelect >> 3) & 0b111), g_HotkeySelect >> 6))
			{
				MessageBox(hwnd, L"选择子窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			//SelectParent的快捷键
			if(g_HotkeySelectParent && !RegisterHotKey(g_MainHwnd, ID_Hotkey_SelectParent, (g_HotkeySelectParent & 0b111) | ((g_HotkeySelectParent >> 3) & 0b111), g_HotkeySelectParent >> 6))
			{
				MessageBox(hwnd, L"选择根窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			//Transparent的快捷键
			if(g_HotkeyTransparent || g_HotkeyMove)
			{
				g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hInstance, 0);
				if(!g_MouseHook)
				{
					wchar_t buf[30];
					wsprintf(buf, L"全局鼠标钩子安装失败! 错误码:%lu\n", GetLastError());
					MessageBox(hwnd, buf, L"提示", MB_OK | MB_ICONERROR);
				}
			}
			//Topmost的快捷键
			if(g_HotkeyTopmost && !RegisterHotKey(g_MainHwnd, ID_Hotkey_Topmost, (g_HotkeyTopmost & 0b111) | ((g_HotkeyTopmost >> 3) & 0b111), g_HotkeyTopmost >> 6))
			{
				MessageBox(hwnd, L"切置顶状态快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static bool Selecting = false;
	auto CreateTray = [&]()->void
	{
		g_nid.cbSize = sizeof(NOTIFYICONDATA);
		g_nid.hWnd = hwnd;
		g_nid.uID = 1;//托盘图标ID
		g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		g_nid.uCallbackMessage = WM_Tray;//自定义消息
		g_nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));
		lstrcpy(g_nid.szTip, TEXT("窗口控制器"));
		Shell_NotifyIcon(NIM_ADD, &g_nid);//添加托盘
	};
	switch(msg)
	{
		case WM_CREATE:
		{
			WM_KeepTray = RegisterWindowMessage(L"TaskbarCreated");
			CreateTray();

			//注册快捷键
			if(g_HotkeyShowMainwindow && !RegisterHotKey(hwnd, ID_Hotkey_Mainwindow, (g_HotkeyShowMainwindow & 0b111) | ((g_HotkeyShowMainwindow >> 3) & 0b111), g_HotkeyShowMainwindow >> 6))
			{
				MessageBox(hwnd, L"显示主窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			if(g_HotkeySelect && !RegisterHotKey(hwnd, ID_Hotkey_Select, (g_HotkeySelect & 0b111) | ((g_HotkeySelect >> 3) & 0b111), g_HotkeySelect >> 6))
			{
				MessageBox(hwnd, L"选择子窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			if(g_HotkeySelectParent && !RegisterHotKey(hwnd, ID_Hotkey_SelectParent, (g_HotkeySelectParent & 0b111) | ((g_HotkeySelectParent >> 3) & 0b111), g_HotkeySelectParent >> 6))
			{
				MessageBox(hwnd, L"选择根窗口快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}
			if(g_HotkeyTransparent || g_HotkeyMove || g_HotkeySize)
			{
				g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, g_hInstance, 0);
				if(!g_MouseHook)
				{
					wchar_t buf[30];
					wsprintf(buf, L"全局鼠标钩子安装失败! 错误码:%lu\n", GetLastError());
					MessageBox(hwnd, buf, L"提示", MB_OK | MB_ICONERROR);
				}
			}
			if(g_HotkeyTopmost && !RegisterHotKey(hwnd, ID_Hotkey_Topmost, (g_HotkeyTopmost & 0b111) | ((g_HotkeyTopmost >> 3) & 0b111), g_HotkeyTopmost >> 6))
			{
				MessageBox(hwnd, L"切置顶状态快捷键注册失败", L"提示", MB_OK | MB_ICONERROR);
			}


			HDC hdc = GetDC(hwnd);
			g_Font= CreateFont(
				-g_ClientSize.cy / 24,
				0,
				0,
				0,
				FW_NORMAL,
				false,
				false,
				false,
				GB2312_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				FF_SWISS,
				TEXT("微软雅黑")
			);
			g_EditFont = CreateFont(
				-g_ClientSize.cy / 27,
				0,
				0,
				0,
				FW_NORMAL,
				false,
				false,
				false,
				GB2312_CHARSET,
				OUT_DEFAULT_PRECIS,
				CLIP_DEFAULT_PRECIS,
				DEFAULT_QUALITY,
				FF_SWISS,
				TEXT("微软雅黑")
			);
			

			HGDIOBJ OldFont = SelectObject(hdc, g_Font);
			GetTextExtentPoint32(hdc, TEXT("中"), 1, &g_FontSize);
			SelectObject(hdc, OldFont);
			ReleaseDC(hwnd, hdc);
			g_LineAnchorY[0] = g_FontSize.cy;
			g_LineAnchorYStep = 2 * g_FontSize.cy;
			for(int i = 1; i < ARRAYSIZE(g_LineAnchorY); ++i)
			{
				g_LineAnchorY[i] = g_LineAnchorY[i - 1] + g_LineAnchorYStep;
			}

			g_ButtonSelect = CreateWindow(
				L"button",
				L"选择",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx,
				g_LineAnchorY[0],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd, (HMENU)ID_BTN_Select, NULL, NULL
			);
			SendMessage(g_ButtonSelect, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonSelect, ButtonSelectProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonSelect, L"按住左键拖动选择任意窗口，按住右键拖动选择顶级窗口", NULL, true, TTI_NONE, NULL, NULL);

			g_ButtonNoactivate = CreateWindow(
				L"button", NULL,
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				g_FontSize.cx * 7.5,
				g_LineAnchorY[0],
				g_FontSize.cx,
				g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Noactivate,
				NULL,
				NULL
			);
			SendMessage(g_ButtonNoactivate, WM_SETFONT, (WPARAM)g_EditFont, 0);
			CreatePointToolTip(hwnd, g_ButtonNoactivate, L"选中后，本工具尝试不再获取焦点", NULL, true, NULL, NULL, NULL);

			g_ButtonShake = CreateWindow(
				L"button",
				L"抖动",
				WS_VISIBLE | WS_CHILD,
				10 * g_FontSize.cx,
				g_LineAnchorY[0],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Shake,
				NULL, NULL
			);
			SendMessage(g_ButtonShake, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonShake, ButtonShakeProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonShake, L"按住右键可以直接显示指示边框", NULL, true, NULL, NULL, NULL);


			g_EditTransparent = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				g_FontSize.cx,
				g_LineAnchorY[1],
				g_FontSize.cx * 6,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_EditTransparent, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonTransparent = CreateWindow(
				L"button",
				L"不透明度",
				WS_VISIBLE | WS_CHILD,
				9 * g_FontSize.cx,
				g_LineAnchorY[1],
				6 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Transparent,
				NULL, NULL
			);
			SendMessage(g_ButtonTransparent, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonTransparent, ButtonTransparentProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonTransparent, L"上下滚轮可以快捷调节透明度", NULL, true, TTI_NONE, NULL, NULL);

			g_EditPosX = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				g_FontSize.cx,
				g_LineAnchorY[2],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(g_EditPosX, WM_SETFONT, (WPARAM)g_EditFont, 0);


			g_EditPosY = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				g_FontSize.cx * 6.5,
				g_LineAnchorY[2],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(g_EditPosY, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonPos = CreateWindow(
				L"button",
				L"位置",
				WS_VISIBLE | WS_CHILD,
				12 * g_FontSize.cx,
				g_LineAnchorY[2],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Pos,
				NULL, NULL
			);
			SendMessage(g_ButtonPos, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonPos, ButtonPosProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonPos, L"右键拖动可以快捷移动窗口", NULL, true, TTI_NONE, NULL, NULL);

			g_EditSizeX = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				g_FontSize.cx,
				g_LineAnchorY[3],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(g_EditSizeX, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_EditSizeY = CreateWindow(
				L"edit",
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER,
				g_FontSize.cx * 6.5,
				g_LineAnchorY[3],
				5 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(g_EditSizeY, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonSize = CreateWindow(
				L"button",
				L"大小",
				WS_VISIBLE | WS_CHILD,
				12 * g_FontSize.cx,
				g_LineAnchorY[3],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd, (HMENU)ID_BTN_Size,
				NULL, NULL
			);
			SendMessage(g_ButtonSize, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonSize, ButtonSizeProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonSize, L"右键可以快捷调节窗口大小", NULL, true, TTI_NONE, NULL, NULL);

			g_StaticExename = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER| ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
				g_FontSize.cx * 23,
				g_LineAnchorY[0],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_StaticExename, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_StaticPid = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
				g_FontSize.cx * 23,
				g_LineAnchorY[1],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL, NULL, NULL
			);
			SendMessage(g_StaticPid, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_StaticClassname = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
				g_FontSize.cx * 23,
				g_LineAnchorY[2],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_StaticClassname, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_EditTitlename = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
				g_FontSize.cx * 23,
				g_LineAnchorY[3],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_EditTitlename, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_EditHandle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_NUMBER,
				g_FontSize.cx * 23,
				g_LineAnchorY[4],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				(HMENU)ID_EDIT_Handle,
				NULL,NULL
			);
			SendMessage(g_EditHandle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_StaticStyle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
				g_FontSize.cx * 23,
				g_LineAnchorY[5],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_StaticStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_StaticExStyle = CreateWindow(
				TEXT("edit"),
				NULL,
				WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL | ES_READONLY,
				g_FontSize.cx * 23,
				g_LineAnchorY[6],
				g_ClientSize.cx - g_FontSize.cx * 23 - g_FontSize.cx * 2,
				g_FontSize.cy * 1.4,
				hwnd,
				NULL,NULL,NULL
			);
			SendMessage(g_StaticExStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonEnable = CreateWindow(
				L"button",
				L"启禁控制",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx,
				g_LineAnchorY[4],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Enable,
				NULL, NULL
			);
			SendMessage(g_ButtonEnable, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonTop = CreateWindow(
				L"button",
				L"置顶选项",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 6,
				g_LineAnchorY[4],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Top,
				NULL, NULL
			);
			SendMessage(g_ButtonTop, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonThrough = CreateWindow(
				L"button",
				L"穿透选项",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 11,
				g_LineAnchorY[4],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Through,
				NULL, NULL
			);
			SendMessage(g_ButtonThrough, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonTitle = CreateWindow(
				L"button",
				L"应用标题",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx,
				g_LineAnchorY[5],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Title,
				NULL, NULL
			);
			SendMessage(g_ButtonTitle, WM_SETFONT, (WPARAM)g_EditFont, 0);
			
			g_ButtonStyle = CreateWindow(
				L"button",
				L"普通风格",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 6,
				g_LineAnchorY[5],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd, (HMENU)ID_BTN_Style, NULL, NULL
			);
			SendMessage(g_ButtonStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonExStyle = CreateWindow(
				L"button",
				L"扩展风格",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 11,
				g_LineAnchorY[5],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_ExStyle,
				NULL, NULL
			);
			SendMessage(g_ButtonExStyle, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonEnum = CreateWindow(
				L"button",
				L"窗口列表",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx,
				g_LineAnchorY[6],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Enum,
				NULL, NULL
			);
			SendMessage(g_ButtonEnum, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonEnum, ButtonEnumProc, 1, 0);
			CreatePointToolTip(hwnd, g_ButtonEnum, L"左键枚举所有顶级窗口，右键枚举当前窗口的子窗口", NULL, true, TTI_NONE, NULL, NULL);


			g_ButtonSetParent = CreateWindow(
				L"button",
				L"窗口从属",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 6,
				g_LineAnchorY[6],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_SetParent,
				NULL, NULL
			);
			SendMessage(g_ButtonSetParent, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonMessage = CreateWindow(
				L"button",
				L"发送信息",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 11,
				g_LineAnchorY[6],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Message,
				NULL, NULL
			);
			SendMessage(g_ButtonMessage, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonAutoRule = CreateWindow(
				L"button",
				L"自动规则",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx,
				g_LineAnchorY[7],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_AutoRule,
				NULL, NULL
			);
			SendMessage(g_ButtonAutoRule, WM_SETFONT, (WPARAM)g_EditFont, 0);
			SetWindowSubclass(g_ButtonAutoRule, ButtonAutoRuleProc, 1, 0);


			g_ButtonAbout = CreateWindow(
				L"button",
				L"关于",
				WS_VISIBLE | WS_CHILD,
				g_ClientSize.cx - g_FontSize.cx * 5,
				g_LineAnchorY[7],
				3 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_About,
				NULL, NULL
			);
			SendMessage(g_ButtonAbout, WM_SETFONT, (WPARAM)g_EditFont, 0);

			g_ButtonHotkey = CreateWindow(
				L"button",
				L"设置热键",
				WS_VISIBLE | WS_CHILD,
				g_FontSize.cx * 6,
				g_LineAnchorY[7],
				4 * g_FontSize.cx,
				1.5 * g_FontSize.cy,
				hwnd,
				(HMENU)ID_BTN_Hotkey,
				NULL, NULL
			);
			SendMessage(g_ButtonHotkey, WM_SETFONT, (WPARAM)g_EditFont, 0);
			break;
		}
		/*case WM_SizeChange:
		{
			HMENU hPopupMenu = CreatePopupMenu();
			if(!hPopupMenu) break;

			// 根据 status 添加第一个项
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MAX, L"最大化");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_NORMAL, L"正常");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_MIN, L"最小化");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_HIDE, L"隐藏");
			// 获取鼠标位置（屏幕坐标）
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			// 显示菜单
			TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

			DestroyMenu(hPopupMenu); // 别忘了销毁
			break;
		}*/
		/*case WM_AutoRuleEnable:
		{
			HMENU hPopupMenu = CreatePopupMenu();
			if(!hPopupMenu) break;

			// 根据 status 添加第一个项
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_AUTORULEENABLE, L"启用");
			AppendMenu(hPopupMenu, MF_STRING, ID_MENU_AUTORULEDISENABLE, L"禁用");
			CheckMenuItem(hPopupMenu, g_AutoRule ? ID_MENU_AUTORULEENABLE : ID_MENU_AUTORULEDISENABLE, MF_BYCOMMAND | MF_CHECKED);
			// 获取鼠标位置（屏幕坐标）
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			// 显示菜单
			TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

			DestroyMenu(hPopupMenu); // 别忘了销毁
			break;
		}*/
		/*case WM_Mouse:
		{
			switch(lParam)
			{
				//滚轮事件
				case 0:
				{
					if(!ModPair(g_HotkeyTransparent))
					{
						break;
					}

					//普通按键匹配检测
					if(g_HotkeyTransparent >> 6)//有普通键
					{
						if(!(GetAsyncKeyState(g_HotkeyTransparent >> 6) & 0x8000))
						{
							break;
						}
					}
					else//无普通键
					{
						bool failed = false;
						for(int vk = 0x08; vk <= 0xFE; ++vk)
						{
							if(vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
							   vk == VK_LSHIFT || vk == VK_RSHIFT ||
							   vk == VK_LCONTROL || vk == VK_RCONTROL ||
							   vk == VK_LMENU || vk == VK_RMENU)
							{
								continue;
							}
							if(GetAsyncKeyState(vk) & 0x8000)
							{
								failed = true;
								break;//有其他键按下
							}
						}
						if(failed)
						{
							break;
						}
					}

					POINT pt;
					GetCursorPos(&pt);
					HWND target = GetAncestor(WindowFromPoint(pt), GA_ROOT);
					if(target)
					{
						int delta = GET_WHEEL_DELTA_WPARAM(wParam);

						INT16 trans;
						if(GetWindowLongPtr(target, GWL_EXSTYLE) & WS_EX_LAYERED)
						{
							BYTE temp;
							GetLayeredWindowAttributes(target, 0, &temp, NULL);
							trans = temp;
						}
						else
						{
							trans = 255;
						}

						if(delta > 0)//up
						{
							if(trans >= 255)
							{
								break;
							}
							trans = trans - trans % 5 + 5;
						}
						else//down
						{
							if(trans <= 0)
							{
								break;
							}
							trans = trans - trans % 5 - 5;
						}
						if(GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
						{
							if(trans <= 55)
							{
								//MessageBox(g_MainHwnd, L"再透明我就要被弄丢了喵~", L">_<", MB_OK | MB_ICONINFORMATION);
								break;
							}
						}
						SetWindowTransparent(target, trans);
					}
					break;
				}
				//左键按下事件
				case 1:
				{
					if(!ModPair(g_HotkeyMove))
					{
						break;
					}

					//普通按键匹配检测
					if(g_HotkeyMove >> 6)//有普通键
					{
						if(!(GetAsyncKeyState(g_HotkeyMove >> 6) & 0x8000))
						{
							break;
						}
					}
					else//无普通键
					{
						bool failed = false;
						for(int vk = 0x08; vk <= 0xFE; ++vk)
						{
							if(vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
							   vk == VK_LSHIFT || vk == VK_RSHIFT ||
							   vk == VK_LCONTROL || vk == VK_RCONTROL ||
							   vk == VK_LMENU || vk == VK_RMENU)
							{
								continue;
							}
							if(GetAsyncKeyState(vk) & 0x8000)
							{
								failed = true;
								break;//有其他键按下
							}
						}
						if(failed)
						{
							break;
						}
					}

					GetCursorPos(&g_HotkeyLastPoint);
					g_HotkeyTargetWindow = GetAncestor(WindowFromPoint(g_HotkeyLastPoint), GA_ROOT);
					if(!IsWindow(g_HotkeyTargetWindow))
					{
						break;
					}
					GetWindowRect(g_HotkeyTargetWindow, &g_HotkeyLastWindow);
					KillTimer(g_MainHwnd, ID_Timer_Size);//防止竞争
					SetTimer(g_MainHwnd, ID_Timer_Move, 16, NULL);
					break;
				}
				//左键抬起事件
				case 2:
				{
					KillTimer(g_MainHwnd, ID_Timer_Move);
					break;
				}
				//右键按下事件
				case 3:
				{
					if(!ModPair(g_HotkeySize))
					{
						break;
					}

					//普通按键匹配检测
					if(g_HotkeySize >> 6)//有普通键
					{
						if(!(GetAsyncKeyState(g_HotkeySize >> 6) & 0x8000))
						{
							break;
						}
					}
					else//无普通键
					{
						bool failed = false;
						for(int vk = 0x08; vk <= 0xFE; ++vk)
						{
							if(vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU ||
							   vk == VK_LSHIFT || vk == VK_RSHIFT ||
							   vk == VK_LCONTROL || vk == VK_RCONTROL ||
							   vk == VK_LMENU || vk == VK_RMENU)
							{
								continue;
							}
							if(GetAsyncKeyState(vk) & 0x8000)
							{
								failed = true;
								break;//有其他键按下
							}
						}
						if(failed)
						{
							break;
						}
					}

					GetCursorPos(&g_HotkeyLastPoint);
					g_HotkeyTargetWindow = GetAncestor(WindowFromPoint(g_HotkeyLastPoint), GA_ROOT);
					if(!IsWindow(g_HotkeyTargetWindow))
					{
						break;
					}
					GetWindowRect(g_HotkeyTargetWindow, &g_HotkeyLastWindow);
					KillTimer(g_MainHwnd, ID_Timer_Move);//防止竞争
					SetTimer(g_MainHwnd, ID_Timer_Size, 16, NULL);
					break;
				}
				//右键抬起事件
				case 4:
				{
					KillTimer(g_MainHwnd, ID_Timer_Size);
					break;
				}
			}
			break;
		}*/
		case WM_TIMER://16ms
		{
			switch(wParam)
			{
				case ID_Timer_Pos://实时位置
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND Target = (HWND)atoi(handle);
					POINT pt;
					GetCursorPos(&pt);
					HWND hParent = GetParent(Target);
					if(hParent && (GetWindowLongPtr(Target, GWL_STYLE) & WS_CHILD))
					{
						MapWindowPoints(NULL, hParent, &pt, 1);
						//ScreenToClient(GetParent(Target), &pt);
					}
					SetWindowPos(Target, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
					break;
				}
				case ID_Timer_LSelect://左键选择
				case ID_Timer_LSelectHotkey://快捷键选择（左键）
				{
					if(wParam == ID_Timer_LSelect && !(GetAsyncKeyState(VK_LBUTTON) & 0x8000))
					{
						KillTimer(hwnd, ID_Timer_LSelect);
						DestroyWindow(g_FrameHWND);
						break;
					}
					POINT pt;
					GetCursorPos(&pt);
					HWND Target = WindowFromPoint(pt);
					if(Target)
					{
						PostMessage(g_FrameHWND, WM_Frame, 0, (LPARAM)Target);
						char handle[HANDLESIZE];
						sprintf(handle, "%llu", (UINT64)Target);
						SetWindowTextA(g_EditHandle, handle);
						FillInformation(Target);
					}
					break;
				}
				case ID_Timer_RSelect://右键选择
				case ID_Timer_RSelectHotkey://快捷键选择（右键）
				{
					if(wParam == ID_Timer_RSelect && !(GetAsyncKeyState(VK_RBUTTON) & 0x8000))
					{
						KillTimer(hwnd, ID_Timer_RSelect);
						DestroyWindow(g_FrameHWND);
						break;
					}
					POINT pt;
					GetCursorPos(&pt);
					HWND Target = GetAncestor(WindowFromPoint(pt), GA_ROOT);
					if(Target)
					{
						PostMessage(g_FrameHWND, WM_Frame, 0, (LPARAM)Target);
						char handle[HANDLESIZE];
						sprintf(handle, "%llu", (UINT64)Target);
						SetWindowTextA(g_EditHandle, handle);
						FillInformation(Target);
					}
					break;
				}
				case ID_Timer_Move://快捷键拖动窗口
				{
					POINT pt;
					GetCursorPos(&pt);
					SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left + (pt.x - g_HotkeyLastPoint.x), g_HotkeyLastWindow.top + (pt.y - g_HotkeyLastPoint.y), 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
					break;
				}
				case ID_Timer_Size://快捷键调整窗口大小
				{

					LONG width = g_HotkeyLastWindow.right - g_HotkeyLastWindow.left;
					LONG height = g_HotkeyLastWindow.bottom - g_HotkeyLastWindow.top;

					LONG x1 = g_HotkeyLastWindow.left + width / 3;
					LONG x2 = g_HotkeyLastWindow.left + width * 2 / 3;
					LONG y1 = g_HotkeyLastWindow.top + height / 3;
					LONG y2 = g_HotkeyLastWindow.top + height * 2 / 3;

					int region = ((g_HotkeyLastPoint.x < x1) ? 0 : (g_HotkeyLastPoint.x < x2 ? 1 : 2)) + 3 * ((g_HotkeyLastPoint.y < y1) ? 0 : (g_HotkeyLastPoint.y < y2 ? 1 : 2));
					POINT pt;
					GetCursorPos(&pt);
					int dx = pt.x - g_HotkeyLastPoint.x;
					int dy = pt.y - g_HotkeyLastPoint.y;
					switch(region)
					{
						//右下角
						case 8:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, 0, 0, width + dx, height + dy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
							break;
						}
						//左下角
						case 6:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left + dx, g_HotkeyLastWindow.top, width - dx, height + dy, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
						//左上角
						case 0:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left + dx, g_HotkeyLastWindow.top + dy, width - dx, height - dy, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
						//右上角
						case 2:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left, g_HotkeyLastWindow.top + dy, width + dx, height - dy, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
						//右边
						case 5:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, 0, 0, width + dx, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
							break;
						}
						//下边
						case 7:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, 0, 0, width, height+dy, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
							break;
						}
						//左边
						case 3:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left + dx, g_HotkeyLastWindow.top, width - dx, height, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
						//上边
						case 1:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left, g_HotkeyLastWindow.top + dy, width, height - dy, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
						//中间
						case 4:
						{
							SetWindowPos(g_HotkeyTargetWindow, NULL, g_HotkeyLastWindow.left - (dx >> 1), g_HotkeyLastWindow.top - (dy >> 1), width + dx, height + dy, SWP_NOACTIVATE | SWP_NOZORDER);
							break;
						}
					}
					break;
				}
			}
			break;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
				case 1:
				{
					ShowWindow(hwnd, SW_NORMAL);
					break;
				}
				case 2:
				{
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
				}
				case ID_MENU_MAX:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					ShowWindow(target, SW_MAXIMIZE); //最大化
					break;
				}
				case ID_MENU_NORMAL:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					ShowWindow(target, SW_RESTORE); //正常
					break;
				}
				case ID_MENU_MIN:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					ShowWindow(target, SW_MINIMIZE); //最小化
					break;
				}
				case ID_MENU_HIDE:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					if(GetAncestor(target, GA_ROOT) == hwnd || GetAncestor(target, GA_ROOTOWNER) == hwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}
					ShowWindow(target, SW_HIDE); //隐藏
					break;
				}
				case ID_MENU_ENABLE:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					EnableWindow(target, true);
					break;
				}
				case ID_MENU_DISABLE:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					if(GetAncestor(target, GA_ROOT) == hwnd || GetAncestor(target, GA_ROOTOWNER) == hwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}
					EnableWindow(target, false);
					break;
				}
				case ID_MENU_TOP:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					SetWindowPos((HWND)atoi(handle), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					break;
				}
				case ID_MENU_NOTOP:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					SetWindowPos((HWND)atoi(handle), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					break;
				}
				case ID_MENU_THROUGH:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					if(GetAncestor(target, GA_ROOT) == hwnd || GetAncestor(target, GA_ROOTOWNER) == hwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}
					SetWindowThrough(target, true, NULL);
					break;
				}
				case ID_MENU_NOTHROUGH:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					SetWindowThrough(target, false, NULL);
					break;
				}
				case ID_MENU_AUTORULEENABLE:
				{
					if(g_AutoRule)
					{
						break;
					}
					WritePrivateProfileString(L"Setting", L"AutoRules", L"1", g_ConfigPath);
					g_AutoRule = true;
					LoadRules();
					g_WindowHook = HOOK
					break;
				}
				case ID_MENU_AUTORULEDISENABLE:
				{
					if(!g_AutoRule)
					{
						break;
					}
					WritePrivateProfileString(L"Setting", L"AutoRules", L"0", g_ConfigPath);
					g_AutoRule = false;
					UnhookWinEvent(g_WindowHook);
					break;
				}
				case ID_BTN_Enable:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					if(!hPopupMenu) break;

					// 根据 status 添加第一个项
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_ENABLE, L"启用");
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_DISABLE, L"禁用");

					// 获取鼠标位置（屏幕坐标）
					POINT pt;
					GetCursorPos(&pt);
					//SetForegroundWindow(hwnd);
					// 显示菜单
					TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

					DestroyMenu(hPopupMenu); // 别忘了销毁
					break;
				}
				case ID_BTN_Shake:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					auto Shake = [](LPVOID Param)->DWORD
					{
						ShakeWindow(*(HWND*)Param);
						EnableWindow(g_ButtonShake, true);
						return 0;
					};
					CloseHandle(CreateThread(NULL, 0, Shake, &target, 0, 0));
					EnableWindow(g_ButtonShake, false);
					break;
				}
				case ID_BTN_Transparent:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					char transparent[5];
					GetWindowTextA(g_EditTransparent, transparent, 5);
					INT32 trans = atoi(transparent);
					if(trans < 0 || trans > 255)
					{
						RECT rect;
						GetWindowRect(g_EditTransparent, &rect);
#if _WIN32_WINNT > _WIN32_WINNT_WINXP
						ShowTooltip(hwnd, NULL, { (rect.left+rect.right)>>1,rect.bottom }, L"透明度只能是0~255", L"提示", true, true, TTI_INFO_LARGE, 2000);
#else
						ShowTooltip(hwnd, NULL, { (rect.left + rect.right) >> 1,rect.bottom }, L"透明度只能是0~255", L"提示", true, true, TTI_INFO, 2000);
#endif

						break;
					}
					if(GetAncestor(target, GA_ROOTOWNER) == g_MainHwnd)
					{
						if(trans <= 55)
						{
							MessageBox(g_MainHwnd, L"再透明我就要被弄丢了喵~", L">_<", MB_OK | MB_ICONINFORMATION);
							break;
						}
					}
					SetWindowTransparent(target, trans);
					break;
				}
				case ID_BTN_Pos:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					char x[11], y[11];
					GetWindowTextA(g_EditPosX, x, ARRAYSIZE(x));
					GetWindowTextA(g_EditPosY, y, ARRAYSIZE(y));
					SetWindowPos(target, NULL, atoi(x), atoi(y), 0, 0, SWP_NOSIZE | SWP_NOZORDER);
					break;
				}
				case ID_BTN_Size:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					char x[11], y[11];
					GetWindowTextA(g_EditSizeX, x, ARRAYSIZE(x));
					GetWindowTextA(g_EditSizeY, y, ARRAYSIZE(y));
					SetWindowPos(target, NULL, 0, 0, atoi(x), atoi(y), SWP_NOMOVE | SWP_NOZORDER);
					break;
				}
				case ID_BTN_Top:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					if(!hPopupMenu) break;

					// 根据 status 添加第一个项
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_TOP, L"置顶");
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_NOTOP, L"取消置顶");

					// 获取鼠标位置（屏幕坐标）
					POINT pt;
					GetCursorPos(&pt);
					// 显示菜单
					TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

					DestroyMenu(hPopupMenu); // 别忘了销毁
					break;
				}
				case ID_BTN_Through:
				{
					HMENU hPopupMenu = CreatePopupMenu();
					if(!hPopupMenu) break;

					// 根据 status 添加第一个项
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_THROUGH, L"设置穿透");
					AppendMenu(hPopupMenu, MF_STRING, ID_MENU_NOTHROUGH, L"取消穿透");

					// 获取鼠标位置（屏幕坐标）
					POINT pt;
					GetCursorPos(&pt);
					// 显示菜单
					TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

					DestroyMenu(hPopupMenu); // 别忘了销毁
					break;
				}
				case ID_EDIT_Handle:
				{
					if(HIWORD(wParam) == EN_CHANGE)
					{
						char handle[HANDLESIZE];
						GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
						HWND target = (HWND)atoi(handle);
						if(IsWindow(target))
						{
							FillInformation(target);
						}
						else
						{
							ResetInformation();
						}
					}
					break;
				}
				case ID_BTN_Title:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						break;
					}
					wchar_t title[MAX_PATH];
					GetWindowText(g_EditTitlename, title, MAX_PATH);
					SetWindowText(target, title);
					break;
				}
				case ID_BTN_Noactivate:
				{
					if(HIWORD(wParam) == BN_CLICKED)
					{
						g_Noactivate = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0);
						DWORD exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
						if(g_Noactivate == BST_CHECKED) exStyle |= WS_EX_NOACTIVATE;
						else exStyle &= (~WS_EX_NOACTIVATE);
						SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
					}
					break;
				}
				case ID_BTN_Style:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						MessageBox(hwnd, L"目标窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					else if(GetAncestor(target, GA_ROOT) == hwnd || GetAncestor(target, GA_ROOTOWNER) == hwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}

					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 7.5 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - 13.5 * g_FontSize.cy;
					rect.right = rect.left + 15 * g_FontSize.cx;
					rect.bottom = rect.top + 37 * g_FontSize.cy;
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_StyleHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - Style",
						L"普通风格",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL,
						NULL,
						NULL
					);
					break;
				}
				case ID_BTN_ExStyle:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						MessageBox(hwnd, L"目标窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					else if(GetAncestor(target, GA_ROOT) == hwnd || GetAncestor(target, GA_ROOTOWNER) == hwnd)
					{
						MessageBox(hwnd, TEXT("请不要这样做喵~"), TEXT(">_<"), MB_OK | MB_ICONINFORMATION);
						break;
					}

					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 8.5 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - 13.5 * g_FontSize.cy;
					rect.right = rect.left + 17 * g_FontSize.cx;
					rect.bottom = rect.top + 37 * g_FontSize.cy;
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_ExStyleHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - ExStyle",
						L"扩展风格",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL,
						NULL,
						NULL
					);
					break;
				}
				case ID_BTN_Enum:
				{
					if(!IsAdmin())
					{
						MessageBox(hwnd, L"当前工具无管理员权限，部分信息无法获取", L"提示", MB_OK | MB_ICONINFORMATION);
					}
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 29 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - 19 * g_FontSize.cy;
					rect.right = rect.left + 58 * g_FontSize.cx;
					rect.bottom = rect.top + 38 * g_FontSize.cy;
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					g_EnumHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - Enum",
						L"窗口列表",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_EnumChild:
				{
					char handle[HANDLESIZE];
					GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
					HWND target = (HWND)atoi(handle);
					if(!IsWindow(target))
					{
						MessageBox(hwnd, L"目标窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					if(!IsAdmin())
					{
						MessageBox(hwnd, L"当前工具无管理员权限，部分信息无法获取", L"提示", MB_OK | MB_ICONINFORMATION);
					}
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 27 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - 19 * g_FontSize.cy;
					rect.right = rect.left + 54 * g_FontSize.cx;
					rect.bottom = rect.top + 38 * g_FontSize.cy;
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					g_EnumChildHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - EnumChild",
						L"子窗口列表",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_SetParent:
				{
					if(!IsAdmin())
					{
						MessageBox(hwnd, L"当前工具无管理员权限，部分操作可能无效", L"提示", MB_OK | MB_ICONINFORMATION);
					}
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 8 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[3] >> 1);
					if(IsWindow(g_SetParentHWND))
					{
						ShowWindow(g_SetParentHWND, SW_NORMAL);
						SetForegroundWindow(g_SetParentHWND);
						SetWindowPos(g_SetParentHWND, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						break;
					}
					rect.right = rect.left + 16 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[3];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_SetParentHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - SetParent",
						L"窗口从属",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_Message:
				{
					if(!IsAdmin())
					{
						MessageBox(hwnd, L"当前工具无管理员权限，部分操作可能无效", L"提示", MB_OK | MB_ICONINFORMATION);
					}
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 11 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[6] >> 1);
					if(IsWindow(g_MessageHWND))
					{
						ShowWindow(g_MessageHWND, SW_NORMAL);
						SetForegroundWindow(g_MessageHWND);
						SetWindowPos(g_MessageHWND, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						break;
					}
					rect.right = rect.left + 22 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[6];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_MessageHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - Message",
						L"发送信息",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_AutoRule:
				{
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 18.5 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[7] >> 1);
					rect.right = rect.left + 37 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[7];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_AutoRuleHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - AutoRule",
						L"自动规则",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_About:
				{
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 20 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[7] >> 1);
					if(IsWindow(g_AboutHWND))
					{
						KillTimer(g_AboutHWND, 1);
						ShowWindow(g_AboutHWND, SW_NORMAL);
						SetForegroundWindow(g_AboutHWND);
						SetWindowPos(g_AboutHWND, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
						break;
					}
					rect.right = rect.left + 40 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[7];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.left < 0)//屏幕左外
					{
						rect.right -= rect.left;
						rect.left = 0;
					}
					g_AboutHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - About",
						L"关于",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
				case ID_BTN_Hotkey:
				{
					RECT rect;
					GetWindowRect(hwnd, &rect);
					rect.left = ((rect.right + rect.left) >> 1) - 13 * g_FontSize.cx;
					rect.top = ((rect.bottom + rect.top) >> 1) - (g_LineAnchorY[9] >> 1);
					rect.right = rect.left + 26 * g_FontSize.cx;
					rect.bottom = rect.top + g_LineAnchorY[9];
					AdjustWindowRectEx(&rect, WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU, false, (g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL));
					if(rect.top < 0)//屏幕上外
					{
						rect.bottom -= rect.top;
						rect.top = 0;
					}
					if(rect.right > g_ScreenSize.cx)//屏幕右外
					{
						rect.left -= rect.right - g_ScreenSize.cx;
						rect.right = g_ScreenSize.cx;
					}
					g_HotkeyHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Window's Handle - Hotkey",
						L"设置热键",
						WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_POPUP | WS_SYSMENU,
						rect.left,
						rect.top,
						rect.right - rect.left,
						rect.bottom - rect.top,
						hwnd,
						NULL, NULL, NULL
					);
					break;
				}
			}
			break;
		}
		case WM_SYSCOMMAND:
		{
			if((wParam & 0xFFF0) == SC_CLOSE)
			{
				ShowWindow(hwnd, SW_HIDE);
				return 0;
			}
			break;
		}
		case WM_HOTKEY:
		{
			switch(wParam)
			{
				case ID_Hotkey_Mainwindow:
				{
					if(!ModPair(g_HotkeyShowMainwindow))
					{
						break;
					}
					ShowWindow(hwnd, SW_NORMAL);
					break;
				}
				case ID_Hotkey_Select:
				{
					if(!ModPair(g_HotkeySelect))
					{
						break;
					}
					Selecting = !Selecting;
					if(Selecting)
					{
						//选择定时器都是互斥的，只能有一个
						KillTimer(g_MainHwnd, ID_Timer_LSelect);
						KillTimer(g_MainHwnd, ID_Timer_RSelect);
						KillTimer(g_MainHwnd, ID_Timer_RSelectHotkey);
						SetTimer(g_MainHwnd, ID_Timer_LSelectHotkey, 16, NULL);
						if(IsWindow(g_FrameHWND))
						{
							DestroyWindow(g_FrameHWND);
						}
						g_FrameHWND = CreateWindowEx(
							WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
							L"Window's Handle - Frame",
							NULL,
							WS_VISIBLE | WS_POPUP,
							0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
							g_MainHwnd,
							NULL, NULL, NULL
						);
					}
					else
					{
						KillTimer(g_MainHwnd, ID_Timer_LSelectHotkey);
						DestroyWindow(g_FrameHWND);
					}
					break;
				}
				case ID_Hotkey_SelectParent:
				{
					if(!ModPair(g_HotkeySelectParent))
					{
						break;
					}
					Selecting = !Selecting;
					if(Selecting)
					{
						//选择定时器都是互斥的，只能有一个
						KillTimer(g_MainHwnd, ID_Timer_LSelect);
						KillTimer(g_MainHwnd, ID_Timer_RSelect);
						KillTimer(g_MainHwnd, ID_Timer_LSelectHotkey);
						SetTimer(g_MainHwnd, ID_Timer_RSelectHotkey, 16, NULL);
						if(IsWindow(g_FrameHWND))
						{
							DestroyWindow(g_FrameHWND);
						}
						g_FrameHWND = CreateWindowEx(
							WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
							L"Window's Handle - Frame",
							NULL,
							WS_VISIBLE | WS_POPUP,
							0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
							g_MainHwnd,
							NULL, NULL, NULL
						);
					}
					else
					{
						KillTimer(g_MainHwnd, ID_Timer_RSelectHotkey);
						DestroyWindow(g_FrameHWND);
					}
					break;
				}
				case ID_Hotkey_Topmost:
				{
					if(!ModPair(g_HotkeyTopmost))
					{
						break;
					}
					POINT pt;
					GetCursorPos(&pt);
					HWND target = GetAncestor(WindowFromPoint(pt), GA_ROOT);
					bool isTopMost = (GetWindowLongPtr(target, GWL_EXSTYLE) & WS_EX_TOPMOST) != 0;
					if(isTopMost)
					{
						SetWindowPos(target, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
						SetForegroundWindow(target);
					}
					else
					{
						SetWindowPos(target, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					}
					break;
				}
			}
			break;
		}
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			SetBkMode(hdc, TRANSPARENT);
			HGDIOBJ OldObj = SelectObject(hdc, g_Font);
			RECT RectDrawText;
			RectDrawText.left = 16 * g_FontSize.cx;
			RectDrawText.right = 22 * g_FontSize.cx;
			RectDrawText.top = g_LineAnchorY[0];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("进程名"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[1];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("PID"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[2];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口类名"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[3];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口标题"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[4];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口句柄"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[5];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("窗口样式"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			RectDrawText.top = g_LineAnchorY[6];
			RectDrawText.bottom = RectDrawText.top + g_LineAnchorYStep;
			DrawText(hdc, TEXT("扩展窗口样式"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

			SelectObject(hdc, OldObj);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO *)lParam;
			mmi->ptMinTrackSize.x = 170; // 设置最小宽度
			mmi->ptMinTrackSize.y = 96; // 设置最小高度
			return 0;
		}
		case WM_WINDOWPOSCHANGING:
		{
			WINDOWPOS *wp = (WINDOWPOS *)lParam;
			wp->flags |= SWP_NOSIZE;
			break;
		}
		case WM_SIZING:
		{
			return 0;
		}
		case WM_Tray:
		{
			if(lParam == WM_RBUTTONUP)
			{
				HMENU hPopupMenu = CreatePopupMenu();
				if(!hPopupMenu) break;
				AppendMenu(hPopupMenu, MF_STRING, 1, L"显示主窗口");
				AppendMenu(hPopupMenu, MF_STRING, 2, L"退出");
				POINT pt;
				GetCursorPos(&pt);
				SetForegroundWindow(hwnd);
				// 显示菜单
				TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);

				DestroyMenu(hPopupMenu); // 别忘了销毁
			}
			else if(lParam == WM_LBUTTONDBLCLK)
			{
				ShowWindow(hwnd, SW_NORMAL);
			}
			break;
		}
		case WM_DESTROY:
		{
			DeleteObject(g_EditFont);
			DeleteObject(g_Font);
			Shell_NotifyIcon(NIM_DELETE, &g_nid);
			UnregisterHotKey(hwnd, ID_Hotkey_Mainwindow);
			UnregisterHotKey(hwnd, ID_Hotkey_Select);
			UnregisterHotKey(hwnd, ID_Hotkey_SelectParent);
			PostQuitMessage(0);
			return 0;
		}
		default:
		{
			if(msg == WM_KeepTray)
			{
				CreateTray();
			}
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevIntance, LPSTR szCmdline, int iCmdShow)
{
	HWND self = GetAncestor(FindWindow(L"Window's Handle", NULL), GA_ROOT);
	if(self != NULL)
	{
		ShowWindow(self, SW_NORMAL);
		SetForegroundWindow(self);
		return 0;
	}
#ifndef _DEBUG
#if _WIN32_WINNT > _WIN32_WINNT_WINXP
	if(IsAdmin())
	{
		InitUIAccess();
	}
#endif
#endif

	//ShowConsole();
	g_hInstance = hInstance;
	argv = CommandLineToArgvW(GetCommandLine(), &argc);
	ReadWriteConfig();
	if(g_AutoRule)
	{
		LoadRules();
		g_WindowHook = HOOK
	}


	//ShowConsole();
	WNDCLASSEX wndclassex = {};
	wndclassex.cbSize = sizeof(WNDCLASSEX); //结构大小
	wndclassex.style = NULL;//窗口类型
	wndclassex.lpfnWndProc = WndProc;//窗口过程(回调函数)
	wndclassex.cbClsExtra = 0;//默认为0
	wndclassex.cbWndExtra = 0;//默认为0
	wndclassex.hInstance = hInstance;//应用程序的实例句柄
	wndclassex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));//所有基于该窗口类的窗口图标
	wndclassex.hCursor = LoadCursor(NULL, IDC_ARROW);//所有基于该窗口类的窗口鼠标指针
	wndclassex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//绘制背景画笔的句柄或颜色值
	wndclassex.lpszMenuName = NULL;//窗口菜单
	wndclassex.lpszClassName = TEXT("Window's Handle");//窗口类名
	wndclassex.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));//与窗口类关联的小图标的句柄
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = AboutProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - About");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = HotkeyProc;
	wndclassex.lpszClassName = TEXT("Window's Handle - Hotkey");
	if(!RegisterClassEx(&wndclassex))
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}


	wndclassex.hbrBackground = NULL;
	wndclassex.lpfnWndProc = FrameProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - Frame");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}


	wndclassex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndclassex.lpfnWndProc = StyleProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - Style");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = ExStyleProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - ExStyle");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = EnumProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - Enum");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = EnumChildWindowProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - EnumChild");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = SetParentProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - SetParent");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = MessageProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - Message");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = AutoRuleProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - AutoRule");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = AutoRuleDetailProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Window's Handle - AutoRuleDetail");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	
	g_ScreenSize = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	SIZE WindowSize = AdjustClientSize(70, 31, 17, true, &g_ClientSize, WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, NULL, NULL);
	g_MainHwnd = CreateWindowEx(
		WS_EX_TOPMOST,// 扩展窗口样式
		TEXT("Window's Handle"),//待注册窗口类名
		g_sTitle,//窗口标题
		WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_POPUP,//窗口样式
		(GetSystemMetrics(SM_CXSCREEN) - WindowSize.cx) >> 1,
		(GetSystemMetrics(SM_CYSCREEN) - WindowSize.cy) >> 1,
		WindowSize.cx,
		WindowSize.cy,
		NULL,//父窗口句柄
		NULL,//菜单句柄
		hInstance,// 关联的模块实例的句柄
		NULL// 需要CREATESTRUCT结构体指针，携带创建窗口时的附加数据
	);

	if(g_isTray)
	{
		ShowWindow(g_MainHwnd, SW_HIDE);
	}
	else if(argc == 1)
	{
		ShowWindow(g_MainHwnd, iCmdShow == SW_MAXIMIZE ? SW_NORMAL : iCmdShow);
		//UpdateWindow(g_MainHwnd);
		SetWindowPos(g_MainHwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	else
	{
		for(int i = 1; i < argc; ++i)
		{
			if(_wcsicmp(L"-tray", argv[i]) == 0)
			{
				ShowWindow(g_MainHwnd, SW_HIDE);
				break;
			}
		}
	}

	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	if(g_AutoRule)UnhookWinEvent(g_WindowHook);
	return 0;
}
