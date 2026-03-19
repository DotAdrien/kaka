#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <fstream>

#include "Globals.h"
#include "GUI.h"
#include "Blink.h"
#include "MinHook.h"

// Détecteur de crash 💥
LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    std::ofstream log("kaka_crash_log.txt", std::ios::app);
    log << "💥 AIE, CRASH DETECTE ! 💥\n";
    log << "Code d'erreur : 0x" << std::hex << ExceptionInfo->ExceptionRecord->ExceptionCode << "\n";
    log << "Adresse : 0x" << ExceptionInfo->ExceptionRecord->ExceptionAddress << "\n";
    log.close();
    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI MainThread(HMODULE hModule) {
    // 0. Activer la protection anti-crash 🛡️
    AddVectoredExceptionHandler(1, CrashHandler);

    // 1. Initialiser MinHook en premier 😎
    if (MH_Initialize() != MH_OK) {
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    // 2. Activer le hook réseau pour le Blink ⚡
    Blink::InitHook();

    HMODULE jvmHandle = GetModuleHandleA("jvm.dll");
    if (!jvmHandle) {
        MH_Uninitialize(); // 🧹 Nettoyage si erreur
        return 0;
    }

    typedef jint(JNICALL* GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);
    auto getVMs = (GetCreatedJavaVMs_t)GetProcAddress(jvmHandle, "JNI_GetCreatedJavaVMs");
    getVMs(&g_vm, 1, nullptr);

    if (g_vm && g_vm->AttachCurrentThread((void**)&g_env, nullptr) == JNI_OK) {
        StartRadarWindow(hModule); // Lance la fenêtre ! 🚀

        // 🛑 Sécurité : on libère les paquets si on ferme le cheat
        Blink::active = false;
        Blink::ReleasePackets();

        g_vm->DetachCurrentThread();
    }

    // 3. Nettoyer les hooks proprement avant de quitter 😌
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    RemoveVectoredExceptionHandler(CrashHandler); // On retire le logger 🧹

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}