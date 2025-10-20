#include "标头.h"

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
bool SetWindowTransparent(HWND &target, UINT8 trans)
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

#if _WIN32_WINNT > _WIN32_WINNT_WINXP
		DWORD dwPathNameSize = sizeof(temp);
		QueryFullProcessImageName(hProcess, 0, temp, &dwPathNameSize);
#else
		GetModuleFileNameEx(hProcess, NULL, temp, ARRAYSIZE(temp));
#endif
		
		CloseHandle(hProcess);
		SetWindowText(g_StaticExename, temp);
	}
	else
	{
		SetWindowText(g_StaticExename, L"[操作错误]");
	}

	_snwprintf(temp, ARRAYSIZE(temp), L"%d", pid);
	SetWindowText(g_StaticPid, temp);

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
	SetWindowText(g_StaticPid, NULL);
	SetWindowText(g_StaticClassname, NULL);
	SetWindowText(g_EditTitlename, NULL);
	SetWindowText(g_StaticStyle, NULL);
	SetWindowText(g_StaticExStyle, NULL);
	SetWindowText(g_EditTransparent, NULL);
}

//加载设置配置
void ReadWriteConfig()
{
	wchar_t ListPath[MAX_PATH];

	getfname(argv[0], NULL, ListPath, NULL, NULL);
	StrCpyNW(g_ConfigPath, ListPath, MAX_PATH);
	StrNCatW(g_ConfigPath, L"\\Config.ini", MAX_PATH);

	


	if(!PathFileExists(g_ConfigPath))
	{
		HANDLE hFile = CreateFileW(g_ConfigPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			const WORD bom = 0xFEFF;
			DWORD written;
			WriteFile(hFile, &bom, sizeof(bom), &written, NULL);
			CloseHandle(hFile);
		}
		WritePrivateProfileString(L"Setting", L"FrameColor", L"FF0000", g_ConfigPath);
		g_FrameColor = RGB(255, 0, 0);
		WritePrivateProfileString(L"Setting", L"FrameWidth", L"4", g_ConfigPath);
		g_FrameWidth = 4;
		WritePrivateProfileString(L"Setting", L"AutoRules", L"0", g_ConfigPath);
		g_AutoRule = false;
		WritePrivateProfileString(L"Setting", L"Tray", L"0", g_ConfigPath);
		g_isTray = false;
	}
	else
	{
		wchar_t temp[7];

		//获取边框颜色
		GetPrivateProfileString(L"Setting", L"FrameColor", L"", temp, ARRAYSIZE(temp), g_ConfigPath);
		if(temp[0] == L'\0')
		{
			WritePrivateProfileString(L"Setting", L"FrameColor", L"FF0000", g_ConfigPath);
			g_FrameColor = RGB(255, 0, 0);
		}
		else
		{
			UINT32 color = wcstol(temp, NULL, 16);
			g_FrameColor = RGB(
				(color >> 16) & 0xFF,
				(color >> 8) & 0xFF,
				color & 0xFF
			);
			if(g_FrameColor == 0x114514)//背景色是臭颜色罢（悲）
			{
				g_FrameColor = RGB(255, 0, 0);
			}
		}

		//获取边框宽度
		g_FrameWidth = GetPrivateProfileInt(L"Setting", L"FrameWidth", 0, g_ConfigPath);
		if(g_FrameWidth <= 0 || g_FrameWidth > 8)
		{
			WritePrivateProfileString(L"Setting", L"FrameWidth", L"4", g_ConfigPath);
			g_FrameWidth = 4;
		}

		//是否启用自动规则
		GetPrivateProfileString(L"Setting", L"AutoRules", L"", temp, ARRAYSIZE(temp), g_ConfigPath);
		if(temp[0] == L'\0')
		{
			WritePrivateProfileString(L"Setting", L"AutoRules", L"0", g_ConfigPath);
			g_AutoRule = false;
		}
		else
		{
			g_AutoRule = wcstol(temp, NULL, 10) == 0 ? false : true;
		}

		//是否隐藏至托盘显示
		GetPrivateProfileString(L"Setting", L"Tray", L"", temp, ARRAYSIZE(temp), g_ConfigPath);
		if(temp[0] == L'\0')
		{
			WritePrivateProfileString(L"Setting", L"Tray", L"0", g_ConfigPath);
			g_isTray = false;
		}
		else
		{
			g_isTray = wcstol(temp, NULL, 10) == 0 ? false : true;
		}
	}
}

//加载规则配置
void LoadRules()
{
	auto EnumIniSections = [](const wstring &iniPath)->vector<UINT16>
	{
		const DWORD BUFFER_SIZE = 8192;
		vector<wchar_t> buffer(BUFFER_SIZE);

		DWORD charsRead = GetPrivateProfileSectionNamesW(buffer.data(), BUFFER_SIZE, iniPath.c_str());
		vector<UINT16> sections;

		if(charsRead == 0)
		{
			// 可能是空文件或读取失败
			return sections;
		}
		啥.reserve(charsRead - 1);
		wchar_t *p = buffer.data();
		while(*p)
		{
			if(_wcsicmp(p, L"Setting") != 0)
			{
				sections.emplace_back(wcstol(p, NULL, 10));
			}
			p += wcslen(p) + 1;             // 跳到下一个字符串
		}

		return sections;
	};
	vector<UINT16> result = EnumIniSections(g_ConfigPath);

	for(auto &x : result)
	{
		V v;
		ZeroMemory(&v, sizeof(V));
		INT16 iTemp;
		wchar_t cTemp[MAX_PATH];
		wchar_t cIndex[4];

		_snwprintf(cIndex, ARRAYSIZE(cIndex), L"%d", x);
		//窗口类名
		GetPrivateProfileString(cIndex, L"ClassName", L"", v.ClassName, ARRAYSIZE(v.ClassName), g_ConfigPath);
		if(v.ClassName[0] == L'\0')//必要值，失败直接放弃
		{
			//free(v);
			continue;
		}

		//进程路径
		GetPrivateProfileString(cIndex, L"Path", L"", v.Path, ARRAYSIZE(v.Path), g_ConfigPath);
		if(v.Path[0] == L'\0')//必要值，失败直接放弃
		{
			//free(v);
			continue;
		}

		//获取掩码
		//UINT16 Flags;
		v.Flags = GetPrivateProfileInt(cIndex, L"Flags", 0, g_ConfigPath);
		if(v.Flags == 0)//啥也没有还干嘛，直接放弃
		{
			//free(v);
			continue;
		}

		//是否再匹配标题
		if(v.Flags & hasTITLE)
		{
			GetPrivateProfileString(cIndex, L"Title", L"", v.Title, ARRAYSIZE(v.Title), g_ConfigPath);
			if(v.Title[0] == L'\0')//错了直接不限制
			{
				v.Flags &= ~hasTITLE;
			}
			else
			{
				;
			}
		}

		//该条目是否启用
		iTemp = GetPrivateProfileInt(cIndex, L"Active", -1, g_ConfigPath);
		//错了直接设置为不激活
		if(iTemp == -1)
		{
			WritePrivateProfileString(cIndex, L"Active", L"0", g_ConfigPath);
			v.Active = 0;
		}
		else
		{
			v.Active = iTemp == 0 ? false : true;
		}

		//是否要透明度
		if(v.Flags & WH_TRANSPARENT)
		{
			v.Trans = GetPrivateProfileInt(cIndex, L"Trans", -1, g_ConfigPath);
			if(v.Trans < 0 || v.Trans > 255)//错了直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_TRANSPARENT;
			}
		}

		//是否要改变位置
		if(v.Flags & WH_POS)
		{
			GetPrivateProfileString(cIndex, L"X", L"", cTemp, ARRAYSIZE(cTemp), g_ConfigPath);
			if(cTemp[0] == L'\0')//读取失败，直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_POS;
			}
			else
			{
				GetPrivateProfileString(cIndex, L"Y", L"", cTemp, ARRAYSIZE(cTemp), g_ConfigPath);
				if(cTemp[0] == L'\0')//读取失败，直接剥夺这个掩码，咱不要了
				{
					v.Flags &= ~WH_POS;
				}
				else
				{
					v.X = GetPrivateProfileInt(cIndex, L"X", NULL, g_ConfigPath);
					v.Y = GetPrivateProfileInt(cIndex, L"Y", NULL, g_ConfigPath);
				}
			}
		}

		//是否要改变大小
		iTemp = GetPrivateProfileInt(cIndex, L"SizeOption", -1, g_ConfigPath);
		if(iTemp < 101 || iTemp > 104)
		{
			v.SizeOption = 102;
		}
		else
		{
			v.SizeOption = iTemp;
		}
		if(v.Flags & WH_SIZE)
		{
			iTemp = GetPrivateProfileInt(cIndex, L"cX", -1, g_ConfigPath);
			if(iTemp == -1)//读取失败，直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_SIZE;
			}
			else
			{
				v.cX = iTemp;
				iTemp = GetPrivateProfileInt(cIndex, L"cY", -1, g_ConfigPath);
				if(iTemp == -1)//读取失败，直接剥夺这个掩码，咱不要了
				{
					v.Flags &= ~WH_SIZE;
				}
				else
				{
					v.cY = iTemp;
				}
			}
		}

		//是否启/禁用该窗口
		if(v.Flags & WH_ENABLE)
		{
			iTemp = GetPrivateProfileInt(cIndex, L"Enable", -1, g_ConfigPath);
			//错了直接剥夺这个掩码，咱不要了
			if(iTemp == -1)
			{
				v.Flags &= ~WH_ENABLE;
			}
			else
			{
				v.Enable = iTemp == 0 ? false : true;
			}
		}

		//是否设置置顶
		if(v.Flags & WH_TOPMOST)
		{
			iTemp = GetPrivateProfileInt(cIndex, L"Topmost", -1, g_ConfigPath);
			//错了直接剥夺这个掩码，咱不要了
			if(iTemp == -1)
			{
				v.Flags &= ~WH_TOPMOST;
			}
			else
			{
				v.Topmost = iTemp == 0 ? false : true;
			}
		}

		//是否设置点击穿透
		if(v.Flags & WH_THROUGH)
		{
			iTemp = GetPrivateProfileInt(cIndex, L"Through", -1, g_ConfigPath);
			//错了直接剥夺这个掩码，咱不要了
			if(iTemp == -1)
			{
				v.Flags &= ~WH_THROUGH;
			}
			else
			{
				v.Through = iTemp == 0 ? false : true;
			}
		}

		//是否需要修改标题
		if(v.Flags & WH_TITLE)
		{
			GetPrivateProfileString(cIndex, L"NewTitle", L"", v.NewTitle, ARRAYSIZE(v.NewTitle), g_ConfigPath);
			if(v.NewTitle[0] == L'\0')//错了直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_TITLE;
			}
			else
			{
				;
			}
		}

		//是否需要修改窗口风格
		if(v.Flags & WH_STYLE)
		{
			GetPrivateProfileString(cIndex, L"Style", L"", cTemp, ARRAYSIZE(cTemp), g_ConfigPath);
			if(cTemp[0] == L'\0')//错了直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_STYLE;
			}
			else
			{
				v.Style = wcstol(cTemp, NULL, 16);
			}
		}

		//是否需要修改扩展窗口风格
		if(v.Flags & WH_EXSTYLE)
		{
			GetPrivateProfileString(cIndex, L"ExStyle", L"", cTemp, ARRAYSIZE(cTemp), g_ConfigPath);
			if(cTemp[0] == L'\0')//错了直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_EXSTYLE;
			}
			else
			{
				v.ExStyle = wcstol(cTemp, NULL, 16);
			}
		}

		//是否需要延时操作
		if(v.Flags & WH_DELAY)
		{
			v.Delay = GetPrivateProfileInt(cIndex, L"Delay", -1, g_ConfigPath);
			if(v.Delay < 0)//错了直接剥夺这个掩码，咱不要了
			{
				v.Flags &= ~WH_DELAY;
			}
		}
		_snwprintf(cTemp, ARRAYSIZE(cTemp), L"%d", v.Flags);
		WritePrivateProfileString(cIndex, L"Flags", cTemp, g_ConfigPath);
		啥.push_back(move(v));
		Class啥[v.ClassName].push_back(&啥.back());
		Index啥[x] = &啥.back();
	}
}

void SaveRules(V *v,UINT16 index)
{
	wchar_t buf[MAX_PATH];
	int i;
	if(index == 0)
	{
		for(i = 1; i < 114514; ++i)
		{
			if(!Index啥.count(i))
			{
				break;
			}
		}
	}
	else
	{
		i = index;
	}

	wchar_t cIndex[5];
	_snwprintf(cIndex, ARRAYSIZE(cIndex), L"%d", i);

	WritePrivateProfileString(cIndex, L"ClassName", v->ClassName, g_ConfigPath);

	WritePrivateProfileString(cIndex, L"Path", v->Path, g_ConfigPath);

	if(v->Flags & hasTITLE)WritePrivateProfileString(cIndex, L"Title", v->Title, g_ConfigPath);

	WritePrivateProfileString(cIndex, L"Active", v->Active ? L"1" : L"0", g_ConfigPath);

	_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->Flags);
	WritePrivateProfileString(cIndex, L"Flags", buf, g_ConfigPath);


	if(v->Flags & WH_TRANSPARENT)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->Trans);
		WritePrivateProfileString(cIndex, L"Trans", buf, g_ConfigPath);
	}

	if(v->Flags & WH_POS)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->X);
		WritePrivateProfileString(cIndex, L"X", buf, g_ConfigPath);
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->Y);
		WritePrivateProfileString(cIndex, L"Y", buf, g_ConfigPath);
	}

	_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->SizeOption);
	WritePrivateProfileString(cIndex, L"SizeOption", buf, g_ConfigPath);
	if(v->Flags & WH_SIZE)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->cX);
		WritePrivateProfileString(cIndex, L"cX", buf, g_ConfigPath);
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->cY);
		WritePrivateProfileString(cIndex, L"cY", buf, g_ConfigPath);
	}

	if(v->Flags & WH_ENABLE)
	{
		WritePrivateProfileString(cIndex, L"Enable", v->Enable ? L"1" : L"0", g_ConfigPath);
	}

	if(v->Flags & WH_TOPMOST)
	{
		WritePrivateProfileString(cIndex, L"Topmost", v->Topmost ? L"1" : L"0", g_ConfigPath);
	}

	if(v->Flags & WH_THROUGH)
	{
		WritePrivateProfileString(cIndex, L"Through", v->Through ? L"1" : L"0", g_ConfigPath);
	}

	if(v->Flags & WH_TITLE)
	{
		WritePrivateProfileString(cIndex, L"NewTitle", v->NewTitle, g_ConfigPath);
	}

	if(v->Flags & WH_STYLE)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"0x%08x", v->Style);
		WritePrivateProfileString(cIndex, L"Style", buf, g_ConfigPath);
	}

	if(v->Flags & WH_EXSTYLE)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"0x%08x", v->ExStyle);
		WritePrivateProfileString(cIndex, L"ExStyle", buf, g_ConfigPath);
	}

	if(v->Flags & WH_DELAY)
	{
		_snwprintf(buf, ARRAYSIZE(buf), L"%d", v->Delay);
		WritePrivateProfileString(cIndex, L"Delay", buf, g_ConfigPath);
	}

	if(i != index)//新的，全部动员
	{
		啥.push_back(move(*v));
		Index啥[i] = &啥.back();
		Class啥[啥.back().ClassName].push_back(&啥.back());
	}
	
	
}

//仅UTF-16类型
bool DeleteIniSection(const wchar_t *iniPath, const wchar_t *sectionToDelete)
{
	FILE *fin = _wfopen(iniPath, L"r, ccs=UNICODE"); // 打开 UTF-16 文件
	if(!fin) return false;

	vector<wstring> lines;
	wstring curline;
	wchar_t ch;
	while((ch = fgetwc(fin)) != WEOF)
	{
		if(ch == L'\r' || ch == L'\n')
		{
			lines.push_back(curline);
			curline.clear();
			if((ch = fgetwc(fin)) == WEOF)continue;
			else curline += ch;
		}
		else
		{
			curline += ch;
		}
	}

	fclose(fin);

	vector<std::wstring> newLines;
	bool inDeleteSection = false;

	// 构造段名字符串，例如 "[MySection]"
	wstring target = L"[";
	target += sectionToDelete;
	target += L"]";

	for(auto &line : lines)
	{

		auto Trim = [](const wstring &s)->wstring
		{
			size_t start = s.find_first_not_of(L" \t\r\n");
			if(start == wstring::npos) return L"";
			size_t end = s.find_last_not_of(L" \t\r\n");
			return s.substr(start, end - start + 1);
		};
		wstring trimmed = Trim(line);

		// 如果当前行是目标段名，进入删除状态
		if(_wcsicmp(trimmed.c_str(), target.c_str()) == 0)
		{
			inDeleteSection = true;
			continue; // 不保留段名
		}

		// 如果当前行是另一个段名，退出删除状态
		if(!trimmed.empty() && trimmed.front() == L'[' && trimmed.back() == L']')
		{
			inDeleteSection = false;
		}

		if(!inDeleteSection)
		{
			newLines.push_back(line);
		}
	}

	FILE *fout = _wfopen(iniPath, L"w, ccs=UNICODE");
	if(!fout) return false;

	for(size_t i = 0; i < newLines.size(); ++i)
	{
		fputws(newLines[i].c_str(), fout);
		fputws(L"\n", fout); // Windows 换行
	}

	fclose(fout);
	return true;
}