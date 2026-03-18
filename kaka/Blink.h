#pragma once
#include <windows.h>
#include <vector>
#include <mutex>
#include <winsock2.h>

namespace Blink {
    extern bool active;
    extern ULONGLONG startTime;
    void InitHook();
    void ReleasePackets();
    void Update();
}