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

// Petite fonction de XOR pour masquer les chaînes
void xor_string(char* data, size_t size, char key) {
    for (size_t i = 0; i < size; i++) data[i] ^= key;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;

    if (MH_Initialize() != MH_OK) {
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    char jvm_name[] = { 0x3f, 0x23, 0x38, 0x7b, 0x31, 0x39, 0x39, 0x00 };
    xor_string(jvm_name, 7, 0x55);
    HMODULE jvmHandle = GetModuleHandleA(jvm_name);
    if (!jvmHandle) {
        MH_Uninitialize();
        FreeLibraryAndExitThread(hModule, 0);
        return 0;
    }

    char proc_name[] = { 0x1f, 0x1b, 0x1c, 0x0a, 0x12, 0x30, 0x21, 0x36, 0x37, 0x34, 0x31, 0x30, 0x31, 0x31, 0x1f, 0x34, 0x23, 0x34, 0x03, 0x18, 0x26, 0x00 };
    xor_string(proc_name, 21, 0x55);

    typedef jint(JNICALL* GetCreatedJavaVMs_t)(JavaVM**, jsize, jsize*);
    auto getVMs = (GetCreatedJavaVMs_t)GetProcAddress(jvmHandle, proc_name);
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