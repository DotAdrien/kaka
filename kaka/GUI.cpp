#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "GUI.h"
#include "Globals.h"
#include <string>

// Procédure de fenêtre ultra-légère
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // On dessine uniquement du texte discret en haut à gauche
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 255, 0)); // Vert discret
            TextOutA(hdc, 5, 5, "System Ready", 12); 
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_ERASEBKGND: return 1; // Empêche le scintillement
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void StartRadarWindow(HMODULE hModule) {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hModule;
    wc.lpszClassName = L"SystemCache_Proc"; // Nom de classe très générique
    RegisterClassEx(&wc);

    // Création d'une fenêtre Overlay invisible/transparente
    HWND hwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        wc.lpszClassName, L"", 
        WS_POPUP, // Pas de bordures, pas de titre
        10, 10, 200, 100, // Petite zone discrète
        NULL, NULL, hModule, NULL
    );

    // Rend la fenêtre transparente (Opacité 150/255)
    SetLayeredWindowAttributes(hwnd, 0, 150, LWA_ALPHA);
    
    // Rendre la fenêtre invisible pour les captures d'écran (Anti-Cheat)
    SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);

    ShowWindow(hwnd, SW_SHOW);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}