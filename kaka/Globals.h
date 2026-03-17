#pragma once
#include <windows.h>
#include <vector>
#include <mutex>
#include <string>
#include <jni.h>
#include <jvmti.h>

struct EntityData { double x, y, z; std::string name; };

struct RadarData {
    double myX = 0, myY = 0, myZ = 0;
    float myYaw = 0;
    std::vector<EntityData> enemies;
    std::vector<EntityData> items;
    std::vector<EntityData> others;
    std::vector<std::string> tabSpectators;
    std::mutex mtx;
};

extern RadarData g_radar;
extern JavaVM* g_vm;
extern JNIEnv* g_env;
extern bool cacheInit;