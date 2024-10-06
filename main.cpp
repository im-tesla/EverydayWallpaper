#include "httplib.h"
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <thread>
#include "defines.h"
#include "bing.h"

class TrayIcon {
public:
    TrayIcon(HWND hwnd, HINSTANCE hInstance, const std::wstring& tooltip)
        : hwnd_(hwnd), hInstance_(hInstance) {
        ZeroMemory(&notifyIconData_, sizeof(NOTIFYICONDATA));
        notifyIconData_.cbSize = sizeof(NOTIFYICONDATA);
        notifyIconData_.hWnd = hwnd_;
        notifyIconData_.uID = ID_TRAY_ICON;
        notifyIconData_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        notifyIconData_.uCallbackMessage = WM_TRAYICON;
        notifyIconData_.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        lstrcpy(notifyIconData_.szTip, tooltip.c_str());

        Shell_NotifyIcon(NIM_ADD, &notifyIconData_);
    }

    ~TrayIcon() {
        Shell_NotifyIcon(NIM_DELETE, &notifyIconData_);
    }

private:
    HWND hwnd_;
    HINSTANCE hInstance_;
    NOTIFYICONDATA notifyIconData_;
};

class ContextMenu {
public:
    ContextMenu() {
        hMenu_ = CreatePopupMenu();
    }

    ~ContextMenu() {
        DestroyMenu(hMenu_);
    }

    void addMenuItem(const std::wstring& itemName, UINT_PTR id, bool enabled = true) {
        AppendMenu(hMenu_, MF_STRING | (enabled ? 0 : MF_GRAYED), id, itemName.c_str());
    }

    void addSeparator() {
        AppendMenu(hMenu_, MF_SEPARATOR, 0, NULL);
    }

    void showMenu(HWND hwnd) {
        POINT point;
        GetCursorPos(&point);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu_, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL);
        PostMessage(hwnd, WM_NULL, 0, 0);
    }

private:
    HMENU hMenu_;
};

std::wstring stringToWString(const std::string& str) {
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()) + 1, 0, 0);
    std::wstring wstr(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), static_cast<int>(str.length()) + 1, &wstr[0], len);
    return wstr;
}

void updateWallpaper() {
    bing.downloadWallpaper();
    bing.setWallpaper();
}

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static TrayIcon* trayIcon = nullptr;
    static ContextMenu* contextMenu = nullptr;

    switch (msg) {
    case WM_CREATE:
        trayIcon = new TrayIcon(hwnd, GetModuleHandle(NULL), L"Everyday Wallpaper");
        contextMenu = new ContextMenu();
        contextMenu->addMenuItem(L"Everyday Wallpaper", 0, false);
        contextMenu->addMenuItem(stringToWString("Current: " + bing.getWallpaperName()), 0, false);
        contextMenu->addSeparator();
        contextMenu->addMenuItem(L"Exit", ID_TRAY_EXIT);

        SetTimer(hwnd, TIMER_ID, UPDATE_INTERVAL, NULL);
        break;
    case WM_TIMER:
        if (wParam == TIMER_ID) {
            updateWallpaper();
        }
        break;
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP) {
            contextMenu->showMenu(hwnd);
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
        KillTimer(hwnd, TIMER_ID);
        delete trayIcon;
        delete contextMenu;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS windowClass = {};
    windowClass.lpfnWndProc = wndProc;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.lpszClassName = L"Everyday Wallpaper";

    RegisterClass(&windowClass);

    HWND hwnd = CreateWindowEx(0, L"Everyday Wallpaper", L"Everyday Wallpaper", 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);

    updateWallpaper();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}