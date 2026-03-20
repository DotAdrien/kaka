#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <jni.h>
#include "MinHook.h"
#include "Globals.h"
#include "DupePoC.h"
#include "Blink.h"
#include "FakeDestroy.h"
#include "pch.h"

// Variables pour éviter le spam des touches
bool kPressed = false;
bool bPressed = false;
bool fPressed = false;

DWORD WINAPI MainThread(LPVOID lpParam) {
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

        // Initialisation du hook réseau pour Blink
        Blink::InitHook();

        while (!(GetAsyncKeyState(VK_END) & 0x8000)) {

            // --- Module Dupe (Touche K) ---
            if (GetAsyncKeyState('K') & 0x8000) {
                if (!kPressed) {
                    DupePoC::Run(g_env, 0, 1);
                    kPressed = true;
                }
            }
            else { kPressed = false; }

            // --- Module Blink (Touche B) ---
            if (GetAsyncKeyState('B') & 0x8000) {
                if (!bPressed) {
                    Blink::active = !Blink::active;
                    if (!Blink::active) Blink::ReleasePackets();
                    bPressed = true;
                }
            }
            else { bPressed = false; }

            // --- Module FakeDestroy (Touche F) ---
            if (GetAsyncKeyState('F') & 0x8000) {
                if (!fPressed) {
                    FakeDestroy::active = !FakeDestroy::active;
                    fPressed = true;
                }
            }
            else { fPressed = false; }

            // Update constant des modules
            Blink::Update();
            FakeDestroy::Update();

            Sleep(10);
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