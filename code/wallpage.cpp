#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include <Tlhelp32.h>
#include <shlwapi.h>
#include<string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>       
#include <io.h>

#define WM_TRAY (WM_USER + 100)
#define WM_TASKBAR_CREATED RegisterWindowMessage(TEXT("TaskbarCreated"))
 
#define APP_NAME	TEXT("wallpape")

 
NOTIFYICONDATA nid;		//托盘属性
HMENU hMenu;			//托盘菜单

int x;
int y;

void * change(void *);		//检测分辨率变化 多线程
//初始化托盘区
void InitTray(HINSTANCE hInstance, HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_TRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_TRAY;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
	lstrcpy(nid.szTip, APP_NAME);

	hMenu = CreatePopupMenu();//生成托盘菜单
	//为托盘菜单添加两个选项

	AppendMenu(hMenu, MF_STRING, ID_SHOW, TEXT("提示"));
	AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("退出"));

	Shell_NotifyIcon(NIM_ADD, &nid);
}

//演示托盘气泡提醒
void ShowTrayMsg()
{
	lstrcpy(nid.szInfoTitle, APP_NAME);
	lstrcpy(nid.szInfo, TEXT("wallpape隐藏至托盘区"));
	nid.uTimeout = 1000;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//隐藏 窗口，类名为WorkerW
BOOL CALLBACK EnumWindowsProc(_In_ HWND hwnd, _In_ LPARAM Lparam)
{
    HWND hDefView = FindWindowExW(hwnd, 0, L"SHELLDLL_DefView", 0);
    if (hDefView != 0){
        // 找它的下一个窗口，类名为WorkerW, 隐藏它
        HWND hWorkerW = FindWindowExW(0, hwnd, L"WorkerW", 0);
        ShowWindow(hWorkerW, SW_HIDE);

        return FALSE;
    }
    return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	STARTUPINFOW si={ 0 };
    PROCESS_INFORMATION pi={ 0 };
	switch (uMsg)
	{
		case WM_TRAY:
			switch(lParam)
			{
				case WM_RBUTTONDOWN:
				{
					//获取鼠标坐标
					POINT pt; GetCursorPos(&pt);

					//解决在菜单外单击左键菜单不消失的问题
					SetForegroundWindow(hWnd);

					//使菜单某项变灰
					//EnableMenuItem(hMenu, ID_SHOW, MF_GRAYED);

					//显示并获取选中的菜单
					int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hWnd, NULL);
					if(cmd == ID_SHOW)
						MessageBox(hWnd, "我们一起找BUG！ ", APP_NAME, MB_OK);
					if(cmd == ID_EXIT)
						PostMessage(hWnd, WM_DESTROY, NULL, NULL);		//发送消息
				}
					break;
				//case WM_LBUTTONDOWN:
				//	MessageBox(hWnd, APP_TIP, APP_NAME, MB_OK);
				//	break;
				case WM_LBUTTONDBLCLK:
					break;
			}
			break;
		case WM_DESTROY:
			CreateProcessW(L"dll\\kill.exe", 0, 0, 0, 0, CREATE_NO_WINDOW , 0, 0, &si, &pi);
			//窗口销毁时删除托盘
			Shell_NotifyIcon(NIM_DELETE, &nid);
			PostQuitMessage(0);
			break;
		case WM_TIMER:
			ShowTrayMsg();
			KillTimer(hWnd, wParam);
			break;
	}
	if (uMsg == WM_TASKBAR_CREATED)
	{
		//系统Explorer崩溃重启时，重新加载托盘
		Shell_NotifyIcon(NIM_ADD, &nid);
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
//搜索 video 文件
wchar_t* listFiles()
{
	char n[20]={0};			//暂存名字
    intptr_t handle;
    _finddata_t findData;
    handle = _findfirst("*.*", &findData);    // 查找目录中的第一个文件
    if (handle == -1)
    {
        return 0;
    }

    do
    {
        if (findData.attrib & _A_SUBDIR && strcmp(findData.name, ".") == 0 && strcmp(findData.name, "..") == 0)
           ;
        else
        {
        	strncpy(n,findData.name,6);
	        if(strcmp(n, "video.")==0)
	        {
	        	wchar_t * dBuf=new wchar_t[200];
	        	MultiByteToWideChar(CP_ACP, 0, findData.name, 260, dBuf, 200);
	        	//wprintf(L"%ls",dBuf);
    			return dBuf;

			}
		}
    } while (_findnext(handle, &findData) == 0);    // 查找目录中的下一个文件

    _findclose(handle);    // 关闭搜索句柄
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	 //得到分辨率
	HDC hdc = GetDC(NULL);
	x = GetDeviceCaps(hdc, DESKTOPHORZRES);
    y = GetDeviceCaps(hdc, DESKTOPVERTRES);
    ReleaseDC(NULL, hdc);

   	wchar_t arr[1024]={0};
   	swprintf(arr,L" .\\%ls  -x %d -y %d -loop 0 -noborder",listFiles(),x,y);

    LPCWSTR lpParameter = arr;
    STARTUPINFOW si={ 0 };
    PROCESS_INFORMATION pi={ 0 };

    //不创建窗口
    if (CreateProcessW(L"dll\\ffplay.exe", (LPWSTR)lpParameter, 0, 0, 0, CREATE_NO_WINDOW , 0, 0, &si, &pi))
    {
        Sleep(2500);
        HWND hProgman = FindWindowW(L"Progman", 0);                  // 找到PM窗口
        SendMessageTimeout(hProgman, 0x52c, 0, 0, 0, 100, 0);        // 给它发特殊消息
        HWND hFfplay = FindWindowW(L"SDL_app", 0);                   // 找到视频窗口
        SetParent(hFfplay, hProgman);                               // 将视频窗口设置为PM的子窗口
        EnumWindows(EnumWindowsProc, 0);                            // 找到第二个workerw窗口并隐藏它
        SetWindowPos(hFfplay,HWND_TOP,0,0,0,0,SWP_NOSIZE);
    }
    else
    {
    	MessageBox(0,"启动失败，缺少文件",0,0);
    	return 0;
	}
	//启动多线程
	pthread_t t0;
	if(pthread_create(&t0, NULL, change, NULL) == -1)
	{
        MessageBox(0,"动态调整分辨率失败",0,0);
        exit(1);
    }



    HWND hWnd;
	MSG msg;
	WNDCLASS wc = { 0 };
	wc.style = NULL;
	wc.hIcon = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WndProc;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = APP_NAME;
	wc.hCursor = NULL;

	if (!RegisterClass(&wc)) return 0;

	hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, APP_NAME, APP_NAME, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	InitTray(hInstance, hWnd);			//实例化托盘
	SetTimer(hWnd, 3, 1000, NULL);		//定时发消息，演示气泡提示功能

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
void * change(void *a){
	int x2;
	int y2;
	HDC hdc = GetDC(NULL);
	STARTUPINFOW si={ 0 };
	PROCESS_INFORMATION pi={ 0 };
    while(1)
	{
        Sleep(1000);

		x2 = GetDeviceCaps(hdc, DESKTOPHORZRES);
    	y2 = GetDeviceCaps(hdc, DESKTOPVERTRES);

    	if(x!=x2 || y!=y2)
    	{
    		x=x2;
    		y=y2;
    		CreateProcessW(L"dll\\kill.exe", 0, 0, 0, 0, CREATE_NO_WINDOW , 0, 0, &si, &pi);
    		Sleep(1000);
    		wchar_t arr[1024]={0};
		   	swprintf(arr,L" .\\%ls  -x %d -y %d -loop 0 -noborder",listFiles(),x,y);

		    LPCWSTR lpParameter = arr;


		    //不创建窗口
		    if (CreateProcessW(L"dll\\ffplay.exe", (LPWSTR)lpParameter, 0, 0, 0, CREATE_NO_WINDOW , 0, 0, &si, &pi))
		    {
		        Sleep(1500);
		        HWND hProgman = FindWindowW(L"Progman", 0);                  // 找到PM窗口
		        SendMessageTimeout(hProgman, 0x52c, 0, 0, 0, 100, 0);        // 给它发特殊消息
		        HWND hFfplay = FindWindowW(L"SDL_app", 0);                   // 找到视频窗口
		        SetParent(hFfplay, hProgman);                               // 将视频窗口设置为PM的子窗口
		        EnumWindows(EnumWindowsProc, 0);                            // 找到第二个workerw窗口并隐藏它
		        SetWindowPos(hFfplay,HWND_TOP,0,0,0,0,SWP_NOSIZE); 
		    }
    		
		}
    }
    return NULL;

}
