#pragma once
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <mutex>


namespace Blink {
    extern bool active;
    extern ULONGLONG startTime;
    void InitHook();
    void ReleasePackets();
    void Update();
}