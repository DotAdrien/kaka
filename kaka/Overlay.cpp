#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

HWND overlayHWND;

// 1. Renommé en OverlayWindowProc 🛠️
LRESULT CALLBACK OverlayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        TextOutA(hdc, 10, 10, "Minimap Invisible", 17);

        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void SetupOverlay() {
    // 2. Change le pointeur ici aussi 👇
    WNDCLASSEXA wc = { sizeof(WNDCLASSEXA), 0, OverlayWindowProc, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, "OverlayClass", nullptr };
    RegisterClassExA(&wc);

    overlayHWND = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        "OverlayClass", "CheatOverlay", WS_POPUP,
        0, 0, 800, 600, nullptr, nullptr, nullptr, nullptr
    );

    SetLayeredWindowAttributes(overlayHWND, RGB(0, 0, 0), 255, LWA_COLORKEY);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(overlayHWND, &margins);
    ShowWindow(overlayHWND, SW_SHOW);
}