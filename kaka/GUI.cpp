#include "GUI.h"
#include "Minimap.h"
#include "Globals.h"
#include "Blink.h"
#include <cmath>

#define MAP_SCALE 4.0f 

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CREATE) SetTimer(hwnd, 1, 16, NULL);
    else if (uMsg == WM_TIMER) {
        UpdateRadarData();
        Blink::Update();
        InvalidateRect(hwnd, NULL, FALSE);
    }
    else if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        int w = rc.right - rc.left; int h = rc.bottom - rc.top;
        int radarW = w / 2;

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, w, h);
        SelectObject(memDC, memBitmap);

        {
            std::lock_guard<std::mutex> lock(g_radar.mtx);

            RECT radarRect = { 0, 0, radarW, h };
            HBRUSH bgBrush = CreateSolidBrush(RGB(10, 10, 10));
            FillRect(memDC, &radarRect, bgBrush);
            DeleteObject(bgBrush);

            RECT infoRect = { radarW, 0, w, h };
            HBRUSH infoBrush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(memDC, &infoRect, infoBrush);
            DeleteObject(infoBrush);

            HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(70, 70, 70));
            SelectObject(memDC, axisPen);
            MoveToEx(memDC, radarW / 2, 0, NULL); LineTo(memDC, radarW / 2, h);
            MoveToEx(memDC, 0, h / 2, NULL); LineTo(memDC, radarW, h / 2);
            MoveToEx(memDC, radarW, 0, NULL); LineTo(memDC, radarW, h);
            DeleteObject(axisPen);

            float rad = g_radar.myYaw * (3.14159265f / 180.0f);
            float cosA = cos(rad), sinA = sin(rad);

            SetBkMode(memDC, TRANSPARENT);

            // Dessin du Blink 🚀
            static bool lKeyPressed = false;
            if (GetAsyncKeyState('L') & 0x8000) {
                if (!lKeyPressed) {
                    Blink::active = !Blink::active;
                    if (Blink::active) Blink::startTime = GetTickCount64();
                    else Blink::ReleasePackets();
                    lKeyPressed = true;
                }
            }
            else {
                lKeyPressed = false;
            }

            if (Blink::active) SetTextColor(memDC, RGB(0, 255, 0));
            else SetTextColor(memDC, RGB(100, 100, 100));
            TextOutA(memDC, 10, 10, "[L] BLINK", 9);

            HBRUSH otherBrush = CreateSolidBrush(RGB(255, 255, 255));
            SetTextColor(memDC, RGB(255, 255, 255));
            for (auto& o : g_radar.others) {
                float rx = o.x - g_radar.myX; float rz = o.z - g_radar.myZ;
                float rotX = rx * cosA + rz * sinA; float rotZ = -rx * sinA + rz * cosA;
                int sx = radarW / 2 - (int)(rotX * MAP_SCALE); int sy = h / 2 - (int)(rotZ * MAP_SCALE);
                if (sx > 0 && sx < radarW && sy > 0 && sy < h) {
                    RECT oRect = { sx - 1, sy - 1, sx + 2, sy + 2 };
                    FillRect(memDC, &oRect, otherBrush);
                    TextOutA(memDC, sx + 5, sy - 5, o.name.c_str(), o.name.length());
                }
            }
            DeleteObject(otherBrush);

            HBRUSH redBrush = CreateSolidBrush(RGB(255, 0, 0));
            HPEN lookPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

            for (auto& e : g_radar.enemies) {
                float rx = e.x - g_radar.myX; float rz = e.z - g_radar.myZ;
                float rotX = rx * cosA + rz * sinA; float rotZ = -rx * sinA + rz * cosA;
                int sx = radarW / 2 - (int)(rotX * MAP_SCALE); int sy = h / 2 - (int)(rotZ * MAP_SCALE);

                if (sx > 0 && sx < radarW && sy > 0 && sy < h) {
                    RECT eRect = { sx - 3, sy - 3, sx + 3, sy + 3 };
                    FillRect(memDC, &eRect, redBrush);
                    TextOutA(memDC, sx + 5, sy - 5, e.name.c_str(), e.name.length());

                    float lookDist = 3.0f;
                    float lx = e.x - sin(e.yaw * 3.14159265f / 180.0f) * lookDist;
                    float lz = e.z + cos(e.yaw * 3.14159265f / 180.0f) * lookDist;

                    float rlx = lx - g_radar.myX; float rlz = lz - g_radar.myZ;
                    float rotLX = rlx * cosA + rlz * sinA; float rotLZ = -rlx * sinA + rlz * cosA;
                    int endX = radarW / 2 - (int)(rotLX * MAP_SCALE);
                    int endY = h / 2 - (int)(rotLZ * MAP_SCALE);

                    SelectObject(memDC, lookPen);
                    MoveToEx(memDC, sx, sy, NULL);
                    LineTo(memDC, endX, endY);
                }
            }
            DeleteObject(redBrush);
            DeleteObject(lookPen);

            HBRUSH meBrush = CreateSolidBrush(RGB(0, 255, 0));
            RECT pRect = { radarW / 2 - 3, h / 2 - 3, radarW / 2 + 3, h / 2 + 3 };
            FillRect(memDC, &pRect, meBrush);

            HPEN myLookPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
            SelectObject(memDC, myLookPen);
            MoveToEx(memDC, radarW / 2, h / 2, NULL);
            LineTo(memDC, radarW / 2, h / 2 - 12);
            DeleteObject(myLookPen);
            DeleteObject(meBrush);

            SetTextColor(memDC, RGB(255, 165, 0));
            TextOutA(memDC, radarW + 10, 10, "--- SPECTATEURS ---", 19);

            int yPos = 35;
            for (const auto& specName : g_radar.tabSpectators) {
                std::string text = "[SPEC] " + specName;
                TextOutA(memDC, radarW + 10, yPos, text.c_str(), text.length());
                yPos += 20;
            }
        }

        BitBlt(hdc, 0, 0, w, h, memDC, 0, 0, SRCCOPY);
        DeleteObject(memBitmap); DeleteDC(memDC); EndPaint(hwnd, &ps);
    }
    else if (uMsg == WM_DESTROY) PostQuitMessage(0);
    else return DefWindowProc(hwnd, uMsg, wParam, lParam);
    return 0;
}

void StartRadarWindow(HMODULE hModule) {
    WNDCLASS wc = { 0 }; wc.lpfnWndProc = WindowProc;
    wc.hInstance = hModule; wc.lpszClassName = L"RadarMap";
    RegisterClass(&wc);

    RECT rect = { 0, 0, 600, 300 };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowEx(0, L"RadarMap", L"Minimap & Tablist 🐱",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        NULL, NULL, hModule, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { TranslateMessage(&msg); DispatchMessage(&msg); }
}