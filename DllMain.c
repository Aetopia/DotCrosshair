#include <Windows.h>

INT g_iSize = 0;
HWND g_hWnd = NULL;

typedef HWND (*CreateWindowInBand)(
    DWORD dwExStyle,
    LPCWSTR lpClassName,
    LPCWSTR lpWindowName,
    DWORD dwStyle,
    int X,
    int Y,
    int nWidth,
    int nHeight,
    HWND hWndParent,
    HMENU hMenu,
    HINSTANCE hInstance,
    LPVOID lpParam,
    DWORD dwBand);

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static UINT s_uTaskbarRestart = 0;
    static NOTIFYICONDATAW Data = {.cbSize = sizeof(NOTIFYICONDATAW),
                                   .uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP,
                                   .uCallbackMessage = WM_USER,
                                   .szTip = L"DotCrosshair"};
    switch (uMsg)
    {

    case WM_CREATE:
        s_uTaskbarRestart = RegisterWindowMessageW(L"TaskbarCreated");
        Data.hWnd = hWnd;
        Data.hIcon = LoadIconW(NULL, IDI_APPLICATION);
        Shell_NotifyIconW(NIM_ADD, &Data);
        break;

    case WM_USER:
        if (lParam == WM_RBUTTONDOWN)
        {
            POINT Point = {};
            SetForegroundWindow(hWnd);
            GetCursorPos(&Point);
            HANDLE hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, IDOK, L"Exit");
            TrackPopupMenu(hMenu, TPM_LEFTBUTTON, Point.x, Point.y, 0, hWnd, NULL);
        }
        break;

    case WM_CLOSE:
    case WM_COMMAND:
        Shell_NotifyIconW(NIM_DELETE, &Data);
        TerminateProcess(GetCurrentProcess(), 0);
        break;

    case WM_STYLECHANGING:
        ((LPSTYLESTRUCT)lParam)->styleNew = wParam == GWL_STYLE
                                                ? WS_VISIBLE | WS_POPUP | WS_BORDER
                                                : WS_EX_NOACTIVATE |
                                                      WS_EX_TRANSPARENT |
                                                      WS_EX_LAYERED |
                                                      WS_EX_TOOLWINDOW;
        break;

    case WM_SETTEXT:
        lParam = (LPARAM)L"DotCrosshair";
        return TRUE;
        break;

    case WM_DISPLAYCHANGE:
        SetWindowPos(hWnd, NULL, 0, 0, 0, 0, 0);
        break;

    case WM_PAINT:
        PAINTSTRUCT Paint = {};
        HDC hDC = BeginPaint(hWnd, &Paint);
        RECT rc = {};
        HBRUSH hbr = GetStockObject(RGB(0, 0, 0));
        GetClientRect(hWnd, &rc);
        FillRect(hDC, &rc, hbr);
        DeleteObject(hbr);
        EndPaint(hWnd, &Paint);
        break;

    case WM_WINDOWPOSCHANGED:
        SetLayeredWindowAttributes(hWnd, 0, 0, LWA_COLORKEY);
        break;

    case WM_WINDOWPOSCHANGING:
        HWND hForegroundWnd = GetForegroundWindow();
        GetClientRect(hForegroundWnd, &rc);
        POINT Point = {((rc.right - rc.left) / 2) + rc.left, ((rc.bottom - rc.top) / 2) + rc.top};
        MapWindowPoints(hForegroundWnd, HWND_DESKTOP, &Point, 1);

        *((PWINDOWPOS)lParam) = (WINDOWPOS){.hwnd = hWnd,
                                            .hwndInsertAfter = HWND_TOPMOST,
                                            .x = Point.x - g_iSize,
                                            .y = Point.y - g_iSize,
                                            .cx = g_iSize * 2,
                                            .cy = g_iSize * 2,
                                            .flags = SWP_SHOWWINDOW |
                                                     SWP_FRAMECHANGED |
                                                     SWP_ASYNCWINDOWPOS |
                                                     SWP_NOACTIVATE};
        break;
    default:
        if (s_uTaskbarRestart)
            Shell_NotifyIconW(NIM_ADD, &Data);
        break;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

VOID WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD dwEvent, HWND hWnd, LONG lIdObject, LONG lIdChild, DWORD dwIdEventThread, DWORD dwMsEventTime)
{
    SetWindowPos(g_hWnd, NULL, 0, 0, 0, 0, 0);
}

DWORD ThreadProc(LPVOID lpParameter)
{
    HMODULE hLibModule = LoadLibraryExW(L"User32.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    CreateWindowInBand fnCreateWindowInBand = (CreateWindowInBand)GetProcAddress(hLibModule, "CreateWindowInBand");
    FreeLibrary(hLibModule);

    if (RegisterClassExW(&((WNDCLASSEXW){
            .cbSize = sizeof(WNDCLASSEXW),
            .hInstance = (HINSTANCE)lpParameter,
            .lpfnWndProc = WndProc,
            .hbrBackground = GetStockObject(BLACK_BRUSH),
            .lpszClassName = L"DotCrosshair"})) &&
        (g_hWnd = fnCreateWindowInBand(
             WS_EX_NOACTIVATE |
                 WS_EX_TRANSPARENT |
                 WS_EX_LAYERED |
                 WS_EX_TOOLWINDOW,
             L"DotCrosshair",
             L"DotCrosshair",
             WS_VISIBLE | WS_POPUP | WS_BORDER,
             0,
             0,
             0,
             0,
             NULL,
             NULL,
             (HINSTANCE)lpParameter,
             NULL,
             2)))
    {

        MSG Msg = {0};
        SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        SetWinEventHook(EVENT_OBJECT_LOCATIONCHANGE, EVENT_OBJECT_LOCATIONCHANGE, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
        WinEventProc(NULL, 0, NULL, OBJID_WINDOW, CHILDID_SELF, 0, 0);
        while (GetMessageW(&Msg, NULL, 0, 0))
            DispatchMessageW(&Msg);
    }

    TerminateProcess(GetCurrentProcess(), 0);
    return EXIT_SUCCESS;
}

BOOL DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        LPWSTR lpFileName = NULL;
        DWORD nSize = 0;
        
        do
            GetModuleFileNameW(hinstDLL, lpFileName = realloc(lpFileName, sizeof(WCHAR) * (nSize += 1)), nSize);
        while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

        for (DWORD dwIndex = nSize; dwIndex < -1; dwIndex -= 1)
            if (lpFileName[dwIndex] == '\\')
            {
                lpFileName[dwIndex + 1] = '\0';
                lpFileName = realloc(lpFileName, sizeof(WCHAR) * (nSize = dwIndex + wcslen(L"DotCrosshair.ini")));
                wcscat(lpFileName, L"DotCrosshair.ini");
                g_iSize = (g_iSize = GetPrivateProfileIntW(L"Settings", L"Size", 3, lpFileName)) > 2 ? g_iSize : 2;
                break;
            }
        free(lpFileName);

        CloseHandle(CreateThread(NULL, 0, ThreadProc, hinstDLL, 0, NULL));
    }
    return TRUE;
}