// ScreenCapture.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "ScreenCapture.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HDC g_srcMemDc; // 源桌面截图
int screenX; // 屏幕的宽
int screenY; // 屏幕的高

RECT rect; // 矩形区域
BOOL isSelect = FALSE; // 是否已经选中了区域
BOOL isDown = FALSE; // 鼠标左键是否按下过

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void ScreenCapture();
void CopyBitmapToCipBoard();

// WunMain 入口函数
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENCAPTURE));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
//  你的窗口长什么样子都是由这个函数决定的
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc    = WndProc; // 窗口过程函数
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENCAPTURE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName = 0; // MAKEINTRESOURCEW(IDC_SCREENCAPTURE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   下面这个函数是用于创建窗口的
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   // 1、窗口类名称
   // 2、窗口标题
   // 3、窗口出来的方式
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MAXIMIZE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
//  CALLBACK(回调函数)、用于给系统调用
//  什么时候调用？当有消息时调用
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // 改变为透明的画刷
    LOGBRUSH brush;
    brush.lbStyle = BS_NULL;
    HBRUSH hBrush = CreateBrushIndirect(&brush);

    // 改变一下笔
    LOGPEN pen;
    POINT ptPen;
    ptPen.x = 2;
    ptPen.y = 2;

    pen.lopnColor = 0x0000FFFF;
    pen.lopnStyle = PS_SOLID;
    pen.lopnWidth = ptPen;

    HPEN hPen = CreatePenIndirect(&pen);

    switch (message) // 消息ID
    {
    case WM_CREATE:
        ScreenCapture();
        break;
        // 里面用于处理一些我们感兴趣的消息
    case WM_LBUTTONDOWN: // 鼠标按下左键
        // MessageBox(hWnd, L"点击左键", L"info", MB_YESNO);
        if (!isSelect)
        {
            POINT pt;
            GetCursorPos(&pt); // 获取光标当前的位置坐标
            rect.left = pt.x;
            rect.top = pt.y;
            rect.right = pt.x;
            rect.bottom = pt.y;
            InvalidateRgn(hWnd, 0, true);

            isDown = TRUE;
        }
        break;
    case WM_LBUTTONUP: // 监听鼠标左键弹起
        if (isDown && !isSelect)
        {
            POINT pt;
            GetCursorPos(&pt); // 获取光标当前的位置坐标
            rect.right = pt.x;
            rect.bottom = pt.y;
            InvalidateRgn(hWnd, 0, true);

            isSelect = TRUE;
            isDown = FALSE;
        }
        break;
    case WM_LBUTTONDBLCLK: // 监听鼠标左键双击，复制选中区域
        if (isSelect)
        {
            CopyBitmapToCipBoard(); //拷贝选中区域到粘贴板
            isSelect = FALSE;
        }
        break;
    case WM_MOUSEMOVE: // 监听鼠标移动
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT: //  屏幕绘制消息，(啥都能画)
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps); // hdc代表了当前窗口
            // TODO: 在此处添加使用 hdc 的任何绘图代码...

            SelectObject(hdc, hBrush);
            SelectObject(hdc, hPen);

            // ScreenCapture();
            BitBlt(hdc, 0, 0, screenX, screenY, g_srcMemDc, 0, 0, SRCCOPY);

            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom); // 画矩形的函数 

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
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


// 获得整个桌面的图像
void ScreenCapture()
{
    HDC disDc = ::CreateDC(L"DISPLAY", 0, 0, 0); //  创建一个DC(设备上下文)，第一个参数固定写法

    // 得到整个桌面的大小(分辨率)
    screenX = GetDeviceCaps(disDc, HORZRES); // 整个桌面水平的宽度
    screenY = GetDeviceCaps(disDc, VERTRES); // 整个桌面垂直的高度

    // 获取桌面截图
    g_srcMemDc = CreateCompatibleDC(disDc); // 创建一个DC(内存设备上下文)，兼容的DC
    // 模拟一张画布
    HBITMAP hbitMap = CreateCompatibleBitmap(disDc, screenX, screenY);
    SelectObject(g_srcMemDc, hbitMap); // 将画布选入内存DC
    // 将桌面截图画到画布上
    BitBlt(g_srcMemDc, 0, 0, screenX, screenY, disDc, 0, 0, SRCCOPY); // 整个桌面截图就已经画到了memDc上
}

// 复制鼠标选中区域到粘贴板
void CopyBitmapToCipBoard()
{
    // 矩形区域的宽和高
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    HDC hSrcDc = ::CreateDC(L"DISPLAY", 0, 0, 0);

    HDC memDc = CreateCompatibleDC(hSrcDc);
    HBITMAP bmp = CreateCompatibleBitmap(hSrcDc, width, height);
    HBITMAP oldbmp = (HBITMAP)SelectObject(memDc, bmp);
    BitBlt(memDc, 0, 0, width, height, hSrcDc, rect.left, rect.top, SRCCOPY);

    HBITMAP newbmp = (HBITMAP)SelectObject(memDc, oldbmp);

    if (OpenClipboard(0))
    {
        EmptyClipboard();
        SetClipboardData(CF_BITMAP, newbmp);
        CloseClipboard();
    }
}







