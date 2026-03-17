#include "SpeedHack.h"
#include "Mappings.h"
#include <windows.h>
#include <cmath>

void RunSpeedHack(jobject myPlayer, float myYaw) {
    if (!getDeltaMovementMethod || !setDeltaMovementMethod || !vec3Class || !vec3Y) return;

    if (GetAsyncKeyState('M') & 0x8000) { // Si la touche M est maintenue 😤

        // On calcule la direction vers laquelle tu regardes 🤓
        float rad = myYaw * (3.14159265f / 180.0f);

        double speed = 1.5; // Modifie cette valeur pour aller plus ou moins vite ! 🚀
        double dx = -sin(rad) * speed;
        double dz = cos(rad) * speed;

        // On récupère ton Y actuel (la gravité) pour que tu ne voles pas bizarrement
        double dy = 0.0;
        jobject vec3Obj = g_env->CallObjectMethod(myPlayer, getDeltaMovementMethod);
        if (vec3Obj) {
            dy = g_env->GetDoubleField(vec3Obj, vec3Y);
            g_env->DeleteLocalRef(vec3Obj);
        }

        // On applique la nouvelle vitesse ! 💥
        g_env->CallVoidMethod(myPlayer, setDeltaMovementMethod, dx, dy, dz);
    }
}