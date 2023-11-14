#include <initguid.h>
#include <Windows.h>
#include <shlobj.h>
#include <knownfolders.h>
#include <stdio.h>

LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CLOSE)
        PostQuitMessage(0);
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, INT nShowCmd)
{
    if (FindWindowW(L"DotCrosshair", NULL))
        return EXIT_SUCCESS;

    LPWSTR lpLibFileName = NULL,
           pszPath = NULL,
           lpApplicationName = NULL;
    PROCESS_INFORMATION ProcessInformation = {};
    STARTUPINFOW StartupInfo = {.cb = sizeof(STARTUPINFOW)};
    DWORD dwSize = 0;
    HANDLE hProcess = GetCurrentProcess();

    do
        lpLibFileName = realloc(lpLibFileName, sizeof(WCHAR) * (dwSize += 1));
    while (!QueryFullProcessImageNameW(hProcess, 0, lpLibFileName, &dwSize));

    for (DWORD dwIndex = dwSize; dwIndex < -1; dwIndex -= 1)
        if (lpLibFileName[dwIndex] == '\\')
        {
            lpLibFileName[dwIndex + 1] = '\0';
            lpLibFileName = realloc(lpLibFileName, sizeof(WCHAR) * (dwSize = dwIndex + wcslen(L"DotCrosshair.dll") + 1));
            wcscat(lpLibFileName, L"DotCrosshair.dll");
            if (FreeLibrary(LoadLibraryExW(lpLibFileName, NULL, DONT_RESOLVE_DLL_REFERENCES)))
            {
                SHGetKnownFolderPath(&FOLDERID_System, 0, NULL, &pszPath);
                lpApplicationName = _malloca(sizeof(WCHAR) * (wcslen(pszPath) + wcslen(L"RuntimeBroker.exe") + 1));
                wcscpy(lpApplicationName, pszPath);
                wcscat(lpApplicationName, L"\\RuntimeBroker.exe");
                CoTaskMemFree(pszPath);

                CreateProcessW(lpApplicationName,
                               NULL,
                               NULL,
                               NULL,
                               FALSE,
                               CREATE_SUSPENDED,
                               NULL,
                               NULL,
                               &StartupInfo,
                               &ProcessInformation);
                LPVOID lpBaseAddress = VirtualAllocEx(ProcessInformation.hProcess, NULL, sizeof(WCHAR) * dwSize, MEM_COMMIT, PAGE_READWRITE);
                WriteProcessMemory(ProcessInformation.hProcess, lpBaseAddress, lpLibFileName, sizeof(WCHAR) * dwSize, NULL);
                CloseHandle(CreateRemoteThread(ProcessInformation.hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, lpBaseAddress, 0, NULL));
                CloseHandle(ProcessInformation.hProcess);
                CloseHandle(ProcessInformation.hThread);
            }
            break;
        }

    return EXIT_SUCCESS;
}