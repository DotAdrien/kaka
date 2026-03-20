#pragma once
#include <windows.h>
#include <memory>

class VMTHook {
public:
    VMTHook(void* instance) {
        base = (uintptr_t**)instance;
        original_vft = *base;
        int size = 0;
        while (original_vft[size]) size++;

        new_vft = std::make_unique<uintptr_t[]>(size + 1);
        memcpy(new_vft.get(), original_vft - 1, sizeof(uintptr_t) * (size + 1));
        *base = new_vft.get() + 1;
    }

    ~VMTHook() {
        *base = original_vft;
    }

    template<typename T>
    void Hook(int index, T new_func) {
        new_vft[index + 1] = (uintptr_t)new_func;
    }

    template<typename T>
    T GetOriginal(int index) {
        return (T)original_vft[index];
    }

private:
    uintptr_t** base;
    uintptr_t* original_vft;
    std::unique_ptr<uintptr_t[]> new_vft;
};