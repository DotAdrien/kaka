#include "FakeDestroy.h"
#include "Mappings.h"
#include <map>
#include <tuple>

std::map<std::tuple<int, int, int>, std::pair<jobject, jobject>> savedBlocks;
bool wasPressed = false;

void FakeDestroy::Update() {
    if (!g_env || !g_vm || !cacheInit) return;

    jobject mcObj = g_env->CallStaticObjectMethod(mcClass, getInstance);
    jobject levelObj = g_env->GetObjectField(mcObj, levelField);

    if (!active) {
        if (!savedBlocks.empty() && levelObj) {
            for (auto const& [coords, data] : savedBlocks) {
                g_env->CallBooleanMethod(levelObj, setBlockMethod, data.first, data.second, 3);
                g_env->DeleteGlobalRef(data.first);
                g_env->DeleteGlobalRef(data.second);
            }
            savedBlocks.clear();
        }
        return;
    }

    jobject hitResultObj = g_env->GetObjectField(mcObj, hitResultField);
    if (!hitResultObj || !levelObj) return;
    if (!g_env->IsInstanceOf(hitResultObj, blockHitResultClass)) return;

    bool isPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
    if (!isPressed) { wasPressed = false; return; }
    if (wasPressed) return;
    wasPressed = true;

    jobject blockPosObj = g_env->CallObjectMethod(hitResultObj, getBlockPosMethod);

    int x = g_env->CallIntMethod(blockPosObj, getBlockX);
    int y = g_env->CallIntMethod(blockPosObj, getBlockY);
    int z = g_env->CallIntMethod(blockPosObj, getBlockZ);
    auto coords = std::make_tuple(x, y, z);

    if (savedBlocks.find(coords) != savedBlocks.end()) return;

    jobject currentState = g_env->CallObjectMethod(levelObj, getBlockStateMethod, blockPosObj);
    jobject airBlock = g_env->GetStaticObjectField(blocksClass, airField);
    jobject airState = g_env->CallObjectMethod(airBlock, defaultStateMethod);

    jobject globalPos = g_env->NewGlobalRef(blockPosObj);
    jobject globalState = g_env->NewGlobalRef(currentState);
    savedBlocks[coords] = { globalPos, globalState };

    g_env->CallBooleanMethod(levelObj, setBlockMethod, blockPosObj, airState, 3);
}