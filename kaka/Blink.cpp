#include "Blink.h"
#include "MinHook.h"
#pragma comment(lib, "ws2_32.lib") // Bibliothèque réseau Windows 🤓

namespace Blink {
    bool active = false;
    ULONGLONG startTime = 0;
    std::mutex mtx;

    struct PacketData {
        SOCKET s;
        std::vector<char> buf;
        int flags;
    };
    std::vector<PacketData> packetQueue;

        // Fonction originale 
    typedef int (WINAPI* SendFn)(SOCKET, const char*, int, int);
    SendFn originalSend = nullptr;

    // Notre Hook 🤫
    int WINAPI HookedSend(SOCKET s, const char* buf, int len, int flags) {
        if (active) {
            std::lock_guard<std::mutex> lock(mtx);
            packetQueue.push_back({ s, std::vector<char>(buf, buf + len), flags });
            return len; // On fait croire au jeu que c'est envoyé ! 🫣
        }
        return originalSend(s, buf, len, flags);
    }

    void ReleasePackets() {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& p : packetQueue) {
            originalSend(p.s, p.buf.data(), p.buf.size(), p.flags);
        }
        packetQueue.clear();
    }

    void Update() {
        // Désactivation après 5 sec ⏳
        if (active && (GetTickCount64() - startTime > 5000)) {
            active = false;
            ReleasePackets();
        }
    }

    void InitHook() {
        MH_Initialize();
        HMODULE hWinsock = GetModuleHandle(L"ws2_32.dll");
        void* sendAddr = (void*)GetProcAddress(hWinsock, "send");
        MH_CreateHook(sendAddr, &HookedSend, (reinterpret_cast<void**>(&originalSend)));
        MH_EnableHook(sendAddr);
    }
}