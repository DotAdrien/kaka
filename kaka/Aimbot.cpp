#include "Aimbot.h"
#include "Mappings.h"

void RunAimbot(double pX, double pY, double pZ, double targetX, double targetY, double targetZ, jobject myPlayer) {
    if (!(GetAsyncKeyState(0xDE) & 0x8000)) return; // Touche ' 💥

    double dx = targetX - pX;
    double dy = targetY - (pY + 1.62);
    double dz = targetZ - pZ;
    double dh = sqrt(dx * dx + dz * dz);

    float yaw = (float)(atan2(-dx, dz) * 180.0 / 3.141592653589793);
    float pitch = (float)(atan2(-dy, dh) * 180.0 / 3.141592653589793);

    g_env->CallVoidMethod(myPlayer, setYRotMethod, yaw);
    g_env->CallVoidMethod(myPlayer, setXRotMethod, pitch);
}