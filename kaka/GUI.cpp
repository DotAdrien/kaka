#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <mutex>
#include "Globals.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ws2_32.lib")

// À appeler dans ta boucle MainThread après UpdateRadarData()
void SendRadarDataUDP() {
    static SOCKET s = INVALID_SOCKET;
    static sockaddr_in dest;

    // Initialisation du socket local
    if (s == INVALID_SOCKET) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        dest.sin_family = AF_INET;
        dest.sin_port = htons(9999);
        dest.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    std::lock_guard<std::mutex> lock(g_radar.mtx);

    // Structure simple pour envoyer les entités
    struct Packet { float x, z; int type; } packet;

    // Envoi de ta position (Type 0)
    packet.x = (float)g_radar.myX;
    packet.z = (float)g_radar.myZ;
    packet.type = 0;
    sendto(s, (char*)&packet, sizeof(packet), 0, (sockaddr*)&dest, sizeof(dest));

    // Envoi des ennemis (Type 1)
    for (auto& e : g_radar.enemies) {
        packet.x = (float)e.x;
        packet.z = (float)e.z;
        packet.type = 1;
        sendto(s, (char*)&packet, sizeof(packet), 0, (sockaddr*)&dest, sizeof(dest));
    }
}