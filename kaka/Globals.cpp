#include "Globals.h"

RadarData g_radar;
JavaVM* g_vm = nullptr;
JNIEnv* g_env = nullptr;
bool cacheInit = false;