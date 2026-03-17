#include "Globals.h"
#include "Minimap.h"

DWORD WINAPI MainThread(HMODULE hModule) {
    HMODULE jvmHandle = GetModuleHandleA("jvm.dll");
    if (!jvmHandle) return 0;

    typedef jint(JNICALL* GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);
    auto getVMs = (GetCreatedJavaVMs_t)GetProcAddress(jvmHandle, "JNI_GetCreatedJavaVMs");
    getVMs(&g_vm, 1, nullptr);

    if (g_vm && g_vm->AttachCurrentThread((void**)&g_env, nullptr) == JNI_OK) {
        StartRadarWindow(hModule); // Lance la fenêtre ! 🚀
        g_vm->DetachCurrentThread();
    }
    FreeLibraryAndExitThread(hModule, 0); return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}