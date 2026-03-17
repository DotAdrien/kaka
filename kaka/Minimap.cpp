#include "Minimap.h"
#include "Globals.h"
#include "Mappings.h"
#include "Aimbot.h"
#include "SpeedHack.h"
#include <algorithm>

#define MAP_SCALE 4.0f 

void UpdateRadarData() {
    if (!g_env || !g_vm) return;
    if (!cacheInit) InitJNICache();
    if (!cacheInit || !getInstance) return;

    jobject mc = g_env->CallStaticObjectMethod(mcClass, getInstance);
    if (!mc) return;

    static ULONGLONG lastTabCheck = 0;
    ULONGLONG now = GetTickCount64();

    // Check toutes les 5 secondes ⏳
    if (now - lastTabCheck >= 5000 && getConnectionMethod && playerInfoMapField && getGameModeMethod && getProfileMethod && getProfileNameMethod && getEnumNameMethod) {
        lastTabCheck = now;
        std::vector<std::string> tempSpecs;
        jobject connection = g_env->CallObjectMethod(mc, getConnectionMethod);

        if (connection) {
            jobject playerMap = g_env->GetObjectField(connection, playerInfoMapField);
            if (playerMap) {
                jobject playerList = g_env->CallObjectMethod(playerMap, mapValuesMethod);
                if (playerList) {
                    jobject iterator = g_env->CallObjectMethod(playerList, iteratorMethod);
                    if (iterator) {
                        while (g_env->CallBooleanMethod(iterator, hasNextMethod)) {
                            jobject playerInfo = g_env->CallObjectMethod(iterator, nextMethod);
                            if (playerInfo) {
                                jobject gameMode = g_env->CallObjectMethod(playerInfo, getGameModeMethod);
                                if (gameMode) {
                                    jstring modeNameStr = (jstring)g_env->CallObjectMethod(gameMode, getEnumNameMethod);
                                    if (modeNameStr) {
                                        const char* modeChars = g_env->GetStringUTFChars(modeNameStr, nullptr);
                                        std::string mode(modeChars);
                                        g_env->ReleaseStringUTFChars(modeNameStr, modeChars);
                                        g_env->DeleteLocalRef(modeNameStr);

                                        if (mode == "SPECTATOR") {
                                            jobject profile = g_env->CallObjectMethod(playerInfo, getProfileMethod);
                                            if (profile) {
                                                jstring nameStr = (jstring)g_env->CallObjectMethod(profile, getProfileNameMethod);
                                                if (nameStr) {
                                                    const char* nameChars = g_env->GetStringUTFChars(nameStr, nullptr);
                                                    tempSpecs.push_back(std::string(nameChars));
                                                    g_env->ReleaseStringUTFChars(nameStr, nameChars);
                                                    g_env->DeleteLocalRef(nameStr);
                                                }
                                                g_env->DeleteLocalRef(profile);
                                            }
                                        }
                                    }
                                    g_env->DeleteLocalRef(gameMode);
                                }
                                g_env->DeleteLocalRef(playerInfo);
                            }
                        }
                        g_env->DeleteLocalRef(iterator);
                    }
                    g_env->DeleteLocalRef(playerList);
                }
                g_env->DeleteLocalRef(playerMap);
            }
            g_env->DeleteLocalRef(connection);
        }

        std::lock_guard<std::mutex> lock(g_radar.mtx);
        g_radar.tabSpectators = tempSpecs;
    }

    jobject level = g_env->GetObjectField(mc, levelField);
    jobject myPlayer = g_env->GetObjectField(mc, myPlayerField);

    if (level && myPlayer && getEntitiesMethod && getX && getY && getZ && getYaw) {
        double pX = g_env->CallDoubleMethod(myPlayer, getX);
        double pY = g_env->CallDoubleMethod(myPlayer, getY);
        double pZ = g_env->CallDoubleMethod(myPlayer, getZ);
        float pYaw = g_env->CallFloatMethod(myPlayer, getYaw);

        jobject iterable = g_env->CallObjectMethod(level, getEntitiesMethod);
        std::vector<EntityData> tempEnemies;
        std::vector<EntityData> tempItems;
        std::vector<EntityData> tempOthers;

        double closestDist = 999999.0;
        double targetX = 0, targetY = 0, targetZ = 0;
        bool foundTarget = false;

        if (iterable) {
            jobject iterator = g_env->CallObjectMethod(iterable, iteratorMethod);
            if (iterator) {
                while (g_env->CallBooleanMethod(iterator, hasNextMethod)) {
                    jobject entityObj = g_env->CallObjectMethod(iterator, nextMethod);
                    if (entityObj) {
                        if (!g_env->IsSameObject(entityObj, myPlayer)) {
                            double eX = g_env->CallDoubleMethod(entityObj, getX);
                            double eY = g_env->CallDoubleMethod(entityObj, getY);
                            double eZ = g_env->CallDoubleMethod(entityObj, getZ);
                            float eYaw = g_env->CallFloatMethod(entityObj, getYaw);
                            std::string eName = GetEntityName(entityObj);

                            if (g_env->IsInstanceOf(entityObj, playerClass)) {
                                tempEnemies.push_back({ eX, eY, eZ, eYaw, eName });
                                double dist = sqrt(pow(eX - pX, 2) + pow(eY - pY, 2) + pow(eZ - pZ, 2));
                                if (dist < closestDist) {
                                    closestDist = dist; targetX = eX; targetY = eY + 1.62; targetZ = eZ; foundTarget = true;
                                }
                            }
                            else if (g_env->IsInstanceOf(entityObj, itemEntityClass)) tempItems.push_back({ eX, eY, eZ, eYaw, eName });
                            else if (g_env->IsInstanceOf(entityObj, armorStandClass)) tempOthers.push_back({ eX, eY, eZ, eYaw, eName });
                        }
                        g_env->DeleteLocalRef(entityObj);
                    }
                }
                g_env->DeleteLocalRef(iterator);
            }
            g_env->DeleteLocalRef(iterable);
        }

        if (foundTarget) RunAimbot(pX, pY, pZ, targetX, targetY, targetZ, myPlayer);

        // NOUVEAU : Le SpeedHack s'active ici ! 🚀🥵
        RunSpeedHack(myPlayer, pYaw);

        {
            std::lock_guard<std::mutex> lock(g_radar.mtx);
            g_radar.myX = pX; g_radar.myY = pY; g_radar.myZ = pZ; g_radar.myYaw = pYaw;
            g_radar.enemies = tempEnemies;
            g_radar.items = tempItems;
            g_radar.others = tempOthers;
        }
    }

    if (myPlayer) g_env->DeleteLocalRef(myPlayer);
    if (level) g_env->DeleteLocalRef(level);
    g_env->DeleteLocalRef(mc);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_CREATE) SetTimer(hwnd, 1, 16, NULL);
    else if (uMsg == WM_TIMER) { UpdateRadarData(); InvalidateRect(hwnd, NULL, FALSE); }
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