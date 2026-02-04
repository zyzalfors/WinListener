#include "winlist.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HHOOK hHook = 0;
    LRESULT res = 0;

    switch(msg) {
        case WM_CREATE:
            InitGui(hWnd);
            break;

        case WM_PAINT:
            RepaintGui(hWnd);
            break;

        case WM_CTLCOLORSTATIC:
            res = PaintCtrl(wParam);
            break;

        case WM_CLOSE:
            BeforeClose(hWnd, hHook);
            break;

        case WM_COMMAND:
            ProcessCheck(hWnd, wParam, &hHook, &KeyProc);
            break;

        case WM_DRAWCLIPBOARD:
            ClipProc(hWnd);
            break;

        default:
            res = DefWindowProc(hWnd, msg, wParam, lParam);
    }

    return res;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow) {
    WNDCLASSEX wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "wc";

    if(!RegisterClassEx(&wc)) return 0;

    HWND hWnd = CreateWindowEx(0, wc.lpszClassName, "WinListener",
                               WS_VISIBLE | WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,
                               CW_USEDEFAULT, CW_USEDEFAULT, 240, 70,
                               NULL, NULL, hInstance, NULL);
    if(!hWnd) return 0;

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}