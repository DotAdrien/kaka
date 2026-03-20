#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fstream>
#include "MinHook.h"
#include "Globals.h"
#include "DupePoC.h" // 📦 Import du module de dupe
#include "VMTHook.h"
#include "pch.h"

void ErasePEHeader(HINSTANCE hModule) {
    DWORD oldProtect;
    VirtualProtect(hModule, 4096, PAGE_READWRITE, &oldProtect);
    ZeroMemory(hModule, 4096);
    VirtualProtect(hModule, 4096, oldProtect, &oldProtect);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    ErasePEHeader((HINSTANCE)lpParam); // On efface les traces 😶
    HMODULE hModule = (HMODULE)lpParam;

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
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
    }
    return TRUE;
}