#include "标头.h"

//全局变量区
HWND g_MainHwnd;
HINSTANCE g_hInstance;
TCHAR g_sTitle[] = TEXT("窗口把柄");
SIZE g_ClientSize;
SIZE g_FontSize;
int g_LineAnchorYStep;
INT g_LineAnchorY[8];
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
HWND g_StaticInstance;
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
LRESULT g_Noactivate;
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
NOTIFYICONDATA nid;
bool isDragging = false;
SIZE g_ScreenSize;
HWND g_FrameHWND = NULL;

//选择按钮的左右键监听
LRESULT CALLBACK ButtonSelectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	switch(msg)
	{
		case WM_LBUTTONDOWN:
		{
			isDragging = true;
			SetCapture(hwnd);
			SetTimer(g_MainHwnd, 2, 10, NULL);
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
				L"Windows' Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				g_MainHwnd,
				NULL, NULL, NULL
			);
			break;
		}
		case WM_LBUTTONUP:
		{
			KillTimer(g_MainHwnd, 2);
			PostMessage(g_MainHwnd, WM_TARGET, 0, 0);
			DestroyWindow(g_FrameHWND);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			isDragging = true;
			SetCapture(hwnd);
			SetTimer(g_MainHwnd, 3, 10, NULL);
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
				L"Windows' Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				g_MainHwnd,
				NULL, NULL, NULL
			);
			break;
		}
		case WM_RBUTTONUP:
		{
			KillTimer(g_MainHwnd, 3);
			PostMessage(g_MainHwnd, WM_TARGET, 0, 0);
			DestroyWindow(g_FrameHWND);
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
			g_FrameHWND = CreateWindowEx(
				WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
				L"Windows' Handle - Frame",
				NULL,
				WS_VISIBLE | WS_POPUP,
				0, 0, g_ScreenSize.cx, g_ScreenSize.cy,
				g_MainHwnd,
				NULL, NULL, NULL
			);
			PostMessage(g_FrameHWND, WM_FRAME, 0, (LPARAM)target);
			break;
		}
		case WM_RBUTTONUP:
		{
			DestroyWindow(g_FrameHWND);
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
			isDragging = true;
			SetCapture(hwnd);
			SetTimer(g_MainHwnd, 1, 10, NULL);
			break;
		}
		case WM_RBUTTONUP:
		{
			if(!IsWindow(target))
			{
				break;
			}
			KillTimer(g_MainHwnd, 1);
			SetCursorPos(cursorPos.x, cursorPos.y);
			PostMessage(g_MainHwnd, WM_POS, 0, 0);
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
		case WM_LBUTTONUP:
		{
			break;
		}
		case WM_RBUTTONUP:
		{
			PostMessage(g_MainHwnd, WM_SIZECHANGE, 0, 0);
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
			char transparent[5];
			GetWindowTextA(g_EditTransparent, transparent, ARRAYSIZE(transparent));
			INT32 trans = atoi(transparent);
			if(delta > 0)//up
			{
				if(trans >= 255)
				{
					break;
				}
				trans = trans - trans % 5 + 5;
			}
			else
			{
				if(trans <= 0)
				{
					break;
				}
				trans = trans - trans % 5 - 5;
			}
			if(!SetWindowTransparent(target, trans))
			{
				break;
			}
			snprintf(transparent, ARRAYSIZE(transparent), "%d", trans);
			SetWindowTextA(g_EditTransparent, transparent);
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

//窗口风格窗口过程
LRESULT CALLBACK StyleProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND EditStyle;
	static HWND ButtonApplyStyle;
	static DWORD StyleValue = 0;
	static unordered_map<HWND, DWORD>buttons;
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
			buttons.reserve(ARRAYSIZE(StyleFlags));
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
				buttons[temp] = x.flag;

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
			if(HIWORD(wParam) != BN_CLICKED)//点击事件
			{
				break;
			}
			if(LOWORD(wParam) == 1145)
			{
				bool selected = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED;
				if(selected)
				{
					StyleValue |= buttons[(HWND)lParam];
				}
				else
				{
					StyleValue &= ~buttons[(HWND)lParam];
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
	static unordered_map<HWND, DWORD>buttons;
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
			buttons.reserve(ARRAYSIZE(ExStyleFlags));
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
				buttons[temp] = x.flag;

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
					ExStyleValue |= buttons[(HWND)lParam];
				}
				else
				{
					ExStyleValue &= ~buttons[(HWND)lParam];
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
				return TRUE; // 返回 TRUE 继续枚举，FALSE 停止
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
				return TRUE; // 返回 TRUE 继续枚举，FALSE 停止
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
			//EnableWindow(g_MainHwnd, false);
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
						//style = GetWindowLongPtr(target, GWL_STYLE);
						//style |= WS_POPUP;
						//SetWindowLongPtr(target, GWL_STYLE, style);
					}
					else if(!IsWindow(parent))
					{
						MessageBox(hwnd, L"新父窗口非法", L">_<", MB_OK | MB_ICONINFORMATION);
						break;
					}
					else
					{
						DWORD style = GetWindowLongPtr(target, GWL_STYLE);
						//style &= ~WS_POPUP;
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
			//EnableWindow(g_MainHwnd, true);
			break;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

//发送信息窗口过程
LRESULT CALLBACK MessageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND target;
	char handle[HANDLESIZE + 1];
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
							MessageBox(hwnd, temp, L"提示", MB_OK | MB_ICONERROR);
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
	constexpr int ID_LINK = 114;
	constexpr float gravity = 0.5;
	static POINT lastPos;
	static DWORD lastTime = 0;
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
			DrawText(hdc, TEXT("版本：1.0.0.0                日期：2025.10.7"), -1, &RectDrawText, DT_LEFT | DT_SINGLELINE);
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
			MoveWindow(hwnd, newX, newY, width, height, TRUE);
			break;
		}
		case WM_ENTERSIZEMOVE:
		{
			GetCursorPos(&lastPos);
			KillTimer(hwnd, 1);
			break;
		}
		case WM_EXITSIZEMOVE:
		{
			SetTimer(hwnd, 1, 8, NULL);
			break;
		}
		case WM_MOVING:
		{
			RECT *pRect = (RECT *)lParam;
			POINT currPos = { pRect->left, pRect->top };

			DWORD now = GetTickCount();
			if(lastTime != 0)
			{
				DWORD deltaTime = now - lastTime;
				if(deltaTime > 0)
				{
					int dx = currPos.x - lastPos.x;
					int dy = currPos.y - lastPos.y;
					horizon_velocity = (double)dx / deltaTime * 32; //帧速
					velocity = (double)dy / deltaTime * 32; //帧速
				}
			}

			lastPos = currPos;
			lastTime = now;
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
			//DeleteObject(hIcon);
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
			hBrBack = CreateSolidBrush(RGB(11, 45, 14));
			hPen = CreatePen(PS_SOLID, 4, RGB(255, 0, 0));
			SetLayeredWindowAttributes(hwnd, RGB(11, 45, 14), 0, LWA_COLORKEY);//绿幕
			break;
		}
		case WM_FRAME:
		{
			if(Target != (HWND)lParam)
			{
				Target = (HWND)lParam;
				InvalidateRect(hwnd, NULL, TRUE);
			}
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


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
		{
			nid.cbSize = sizeof(NOTIFYICONDATA);//结构的大小
			nid.hWnd = hwnd;//接收与通知区域中图标关联的通知的窗口句柄
			nid.uID = 1;//托盘图标ID
			nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;//想干嘛
			nid.uCallbackMessage = WM_TRAY;// 自定义消息，请在消息循环内处理WM_TRAY消息
			nid.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON1));//图标
			lstrcpy(nid.szTip, TEXT("窗口控制器"));//定义托盘文本
			Shell_NotifyIcon(NIM_ADD, &nid);//添加托盘


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
				hwnd, (HMENU)ID_BTN_SELECT, NULL, NULL
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
				(HMENU)ID_BTN_SHAKE,
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
				(HMENU)ID_BTN_TRANSPARENT,
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
				(HMENU)ID_BTN_POS,
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
				hwnd, (HMENU)ID_BTN_SIZE,
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

			g_StaticInstance = CreateWindow(
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
			SendMessage(g_StaticInstance, WM_SETFONT, (WPARAM)g_EditFont, 0);

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
				(HMENU)ID_BTN_ENABLE,
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
				(HMENU)ID_BTN_TOP,
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
				(HMENU)ID_BTN_THROUGH,
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
			break;
		}
		case WM_TARGET:
		{
			if(isDragging)
			{
				isDragging = false;
				ReleaseCapture();  // 释放鼠标捕获
			}
			break;
		}
		case WM_POS:
		{
			if(isDragging)
			{
				isDragging = false;
				ReleaseCapture();  // 释放鼠标捕获
				char handle[HANDLESIZE];
				wchar_t Temp[21];
				GetWindowTextA(g_EditHandle, handle, ARRAYSIZE(handle));
				RECT rect;
				GetWindowRect((HWND)atoi(handle), &rect);
				wsprintf(Temp, L"%ld", rect.left);
				SetWindowText(g_EditPosX, Temp);
				wsprintf(Temp, L"%ld", rect.top);
				SetWindowText(g_EditPosY, Temp);
			}
			break;
		}
		case WM_SIZECHANGE:
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
		}
		case WM_TIMER:
		{
			//实时位置
			if(wParam == 1)
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
			}
			//左键选择
			else if(wParam == 2)
			{
				POINT pt;
				GetCursorPos(&pt);
				HWND Target = WindowFromPoint(pt);
				if(Target)
				{
					PostMessage(g_FrameHWND, WM_FRAME, 0, (LPARAM)Target);
					char handle[HANDLESIZE];
					//设置句柄
					sprintf(handle, "%llu", (UINT64)Target);
					SetWindowTextA(g_EditHandle, handle);
					FillInformation(Target);
				}
			}
			//右键选择
			else if(wParam == 3)
			{
				POINT pt;
				GetCursorPos(&pt);
				HWND Target = GetAncestor(WindowFromPoint(pt), GA_ROOT);
				if(Target)
				{
					PostMessage(g_FrameHWND, WM_FRAME, 0, (LPARAM)Target);
					char handle[HANDLESIZE];
					//设置句柄
					sprintf(handle, "%llu", (UINT64)Target);
					SetWindowTextA(g_EditHandle, handle);
					FillInformation(Target);
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
					HWND Target = (HWND)atoi(handle);
					if(GetParent(Target))
					{
						SetWindowPos(Target, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					}
					else
					{
						SetWindowPos(Target, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
					}
					
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
				case ID_BTN_ENABLE:
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
				case ID_BTN_SHAKE:
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
				case ID_BTN_TRANSPARENT:
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
					SetWindowTransparent(target, trans);
					break;
				}
				case ID_BTN_POS:
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
				case ID_BTN_SIZE:
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
				case ID_BTN_TOP:
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
				case ID_BTN_THROUGH:
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
					g_StyleHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - Style",
						L"选择风格",
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
					g_ExStyleHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - ExStyle",
						L"选择风格",
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
					g_EnumHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - Enum",
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
					g_EnumChildHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - EnumChild",
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
					g_SetParentHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - SetParent",
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
					g_MessageHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - Message",
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
					g_AboutHWND = CreateWindowEx(
						(g_Noactivate == BST_CHECKED ? WS_EX_NOACTIVATE : NULL),
						L"Windows' Handle - About",
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
			}
			break;
		}
		case WM_SYSCOMMAND:
		{
			if((wParam & 0xFFF0) == SC_MINIMIZE)
			{
				if(g_Noactivate == BST_CHECKED)
				{
					ShowWindow(hwnd, SW_HIDE);
					return 0;
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
			DrawText(hdc, TEXT("实例句柄"), -1, &RectDrawText, DT_RIGHT | DT_SINGLELINE);

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
		case WM_TRAY:
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
			Shell_NotifyIcon(NIM_DELETE, &nid);
			PostQuitMessage(0);
			return 0;
		}
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevIntance, LPSTR szCmdline, int iCmdShow)
{
	HWND self = GetAncestor(FindWindow(L"Windows' Handle", NULL), GA_ROOT);
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
	g_hInstance = hInstance;
	//全局资源加载区



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
	wndclassex.lpszClassName = TEXT("Windows' Handle");//窗口类名
	wndclassex.hIconSm = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_ICON1));//与窗口类关联的小图标的句柄
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = AboutProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - About");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.hbrBackground = NULL;
	wndclassex.lpfnWndProc = FrameProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - Frame");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}


	wndclassex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wndclassex.lpfnWndProc = StyleProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - Style");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = ExStyleProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - ExStyle");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = EnumProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - Enum");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = EnumChildWindowProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - EnumChild");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = SetParentProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - SetParent");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	wndclassex.lpfnWndProc = MessageProc;//窗口过程(回调函数)
	wndclassex.lpszClassName = TEXT("Windows' Handle - Message");//窗口类名
	if(!RegisterClassEx(&wndclassex))//注册并检验窗口
	{
		MessageBox(NULL, TEXT("这个程序需要在Windows NT才能执行！"), g_sTitle, MB_ICONERROR);
		return 0;
	}

	
	g_ScreenSize = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };
	SIZE WindowSize = AdjustClientSize(70, 31, 17, true, &g_ClientSize, WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, NULL, NULL);
	g_MainHwnd = CreateWindowEx(
		WS_EX_TOPMOST,// 扩展窗口样式
		TEXT("Windows' Handle"),//待注册窗口类名
		g_sTitle,//窗口标题
		WS_OVERLAPPED | WS_VISIBLE | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_POPUP,//窗口样式
		(GetSystemMetrics(SM_CXSCREEN) - WindowSize.cx) >> 1,
		(GetSystemMetrics(SM_CYSCREEN) - WindowSize.cy) >> 1,
		WindowSize.cx,
		WindowSize.cy,
		NULL,//父窗口句柄
		NULL,//菜单句柄
		hInstance,// 关联的模块实例的句柄
		NULL// 需要CREATESTRUCT结构体指针，携带创建窗口时的附加数据
	);
	ShowWindow(g_MainHwnd, iCmdShow == SW_MAXIMIZE ? SW_NORMAL : iCmdShow);
	UpdateWindow(g_MainHwnd);
	SetWindowPos(g_MainHwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	MSG msg;
	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}
