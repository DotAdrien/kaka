#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include "MinHook.h"
#include "Globals.h"
#include "DupePoC.h" // 📦 Import du module de dupe

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    DWORD code = ExceptionInfo->ExceptionRecord->ExceptionCode;
    if (code == 0x4001000a || code == 0x40010006 || code == 0x406D1388) return EXCEPTION_CONTINUE_SEARCH;

    std::ofstream log("kaka_crash_log.txt", std::ios::app);
    log << "💥 CRASH DETECTE DANS LE MODULE DUPE : 0x" << std::hex << code << "\n";
    log.close();
    return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI MainThread(HMODULE hModule) {
    AddVectoredExceptionHandler(1, CrashHandler);

    if (MH_Initialize() != MH_OK) {
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    HMODULE jvmHandle = GetModuleHandleA("jvm.dll");
    if (!jvmHandle) {
        MH_Uninitialize();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    typedef jint(JNICALL* GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);
    auto getVMs = (GetCreatedJavaVMs_t)GetProcAddress(jvmHandle, "JNI_GetCreatedJavaVMs");
    getVMs(&g_vm, 1, nullptr);

    if (g_vm && g_vm->AttachCurrentThread((void**)&g_env, nullptr) == JNI_OK) {

        bool kPressed = false;
        // 🔄 Boucle invisible pour le module
        while (!(GetAsyncKeyState(VK_END) & 0x8000)) {

            if (GetAsyncKeyState('K') & 0x8000) {
                if (!kPressed) {
                    // 🚀 Execution du module de duplication
                    DupePoC::Run(g_env, 0, 1);
                    kPressed = true;
                }
            }
            else {
                kPressed = false;
            }

            Sleep(10); // 💤 Petite pause pour le CPU
        }

        g_vm->DetachCurrentThread();
    }

    MH_Uninitialize();
    RemoveVectoredExceptionHandler(CrashHandler);
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