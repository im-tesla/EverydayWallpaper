#include "httplib.h"
#include <windows.h>
#include <shellapi.h>
#include "defines.h"
#include "bing.h"

HINSTANCE hInst;
NOTIFYICONDATA nid;
HMENU hMenu;

std::wstring stringToWString(const std::string& str) {
    int len;
    int strLength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), strLength, 0, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), strLength, &wstr[0], len);
    return wstr;
}

void createTrayIcon(HWND hwnd) {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = ID_TRAY_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    lstrcpy(nid.szTip, TEXT("Tray Icon Example"));

    Shell_NotifyIcon(NIM_ADD, &nid);
}

void removeTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

void showContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
    PostMessage(hwnd, WM_NULL, 0, 0);
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
            showContextMenu(hwnd);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
            PostQuitMessage(0);
            break;
        }
        break;
    case WM_DESTROY:
        removeTrayIcon();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void createContextMenu() {
    hMenu = CreatePopupMenu();
	std::string name = "Current: " + bing.getWallpaperName();
    AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, TEXT("Everyday Wallpaper"));
    AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, stringToWString(name).c_str());
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));
}

void detachedThread() {
    while (true) {
        req::getWallpaper();
        bing.downloadWallpaper();
        bing.setWallpaper();
        Sleep(1800000);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {};
    wc.lpfnWndProc = wndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"Everyday Wallpaper";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"Everyday Wallpaper", L"Everyday Wallpaper", 0, 0, 0, 0, 0, NULL, NULL, wc.hInstance, NULL);

    createContextMenu();
    createTrayIcon(hwnd);

    std::thread t(detachedThread);
    t.detach();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyMenu(hMenu);

    return 0;
}
