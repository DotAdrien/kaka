#pragma once
#include <jni.h>

class DupePoC {
public:
    static void Run(JNIEnv* env, int gunSlot, int attachSlot);
};