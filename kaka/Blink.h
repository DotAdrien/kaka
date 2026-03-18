#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <vector>
#include <mutex>

namespace Blink {
    extern bool active;
    extern ULONGLONG startTime;

    struct QueuedPacket {
        SOCKET s;
        std::vector<char> buffer;
        DWORD flags;
    };

    extern std::vector<QueuedPacket> packetQueue;
    extern std::mutex queueMutex;

    void InitHook();
    void ReleasePackets();
    void Update();
}