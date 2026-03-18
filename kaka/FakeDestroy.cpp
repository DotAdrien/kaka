#include "FakeDestroy.h"
#include "Globals.h"
#include <map>

// Stockage sécurisé des blocs pour pouvoir les remettre 📦
std::map<jobject, jobject> savedBlocks;

void FakeDestroy::Update() {
    if (!g_env || !g_vm) return;

    jclass mcClass = g_env->FindClass("net/minecraft/client/Minecraft");
    jclass blockHitResultClass = g_env->FindClass("net/minecraft/world/phys/BlockHitResult");
    jclass levelClass = g_env->FindClass("net/minecraft/world/level/Level");
    jclass blockClass = g_env->FindClass("net/minecraft/world/level/block/Block");
    jclass blocksClass = g_env->FindClass("net/minecraft/world/level/block/Blocks");

    jmethodID getInstanceMethod = g_env->GetStaticMethodID(mcClass, "getInstance", "()Lnet/minecraft/client/Minecraft;");
    jobject mcObj = g_env->CallStaticObjectMethod(mcClass, getInstanceMethod);

    jfieldID levelField = g_env->GetFieldID(mcClass, "f_91073_", "Lnet/minecraft/client/multiplayer/ClientLevel;"); // Minecraft.level
    jobject levelObj = g_env->GetObjectField(mcObj, levelField);

    // Si le cheat est désactivé, on fait réapparaître les blocs 🔙
    if (!active) {
        if (!savedBlocks.empty() && levelObj) {
            jmethodID setBlockMethod = g_env->GetMethodID(levelClass, "m_7731_", "(Lnet/minecraft/core/BlockPos;Lnet/minecraft/world/level/block/state/BlockState;I)Z");
            for (auto const& [pos, state] : savedBlocks) {
                g_env->CallBooleanMethod(levelObj, setBlockMethod, pos, state, 3);
                g_env->DeleteGlobalRef(pos);
                g_env->DeleteGlobalRef(state);
            }
            savedBlocks.clear();
        }
        return;
    }

    jfieldID hitResultField = g_env->GetFieldID(mcClass, "f_91077_", "Lnet/minecraft/world/phys/HitResult;");
    jobject hitResultObj = g_env->GetObjectField(mcObj, hitResultField);

    if (!hitResultObj || !levelObj) return;

    // On s'assure qu'on vise bien un bloc 🎯
    if (!g_env->IsInstanceOf(hitResultObj, blockHitResultClass)) return;

    // On casse le bloc seulement si clic gauche appuyé 💥
    if (!(GetAsyncKeyState(VK_LBUTTON) & 0x8000)) return;

    jmethodID getBlockPosMethod = g_env->GetMethodID(blockHitResultClass, "m_82425_", "()Lnet/minecraft/core/BlockPos;");
    jobject blockPosObj = g_env->CallObjectMethod(hitResultObj, getBlockPosMethod);

    // Récupérer l'état actuel du bloc avant de le casser 🕵️‍♂️
    jmethodID getBlockStateMethod = g_env->GetMethodID(levelClass, "m_8055_", "(Lnet/minecraft/core/BlockPos;)Lnet/minecraft/world/level/block/state/BlockState;");
    jobject currentState = g_env->CallObjectMethod(levelObj, getBlockStateMethod, blockPosObj);

    // Récupérer l'air 💨
    jfieldID airField = g_env->GetStaticFieldID(blocksClass, "f_50016_", "Lnet/minecraft/world/level/block/Block;");
    jobject airBlock = g_env->GetStaticObjectField(blocksClass, airField);
    jmethodID defaultStateMethod = g_env->GetMethodID(blockClass, "m_49966_", "()Lnet/minecraft/world/level/block/state/BlockState;"); // defaultBlockState()
    jobject airState = g_env->CallObjectMethod(airBlock, defaultStateMethod);

    // Sauvegarder (GlobalRef pour pas crash avec le Garbage Collector) 🧠
    jobject globalPos = g_env->NewGlobalRef(blockPosObj);
    jobject globalState = g_env->NewGlobalRef(currentState);
    savedBlocks[globalPos] = globalState;

    // Remplacer le bloc par de l'air instantanément 🛠️
    jmethodID setBlockMethod = g_env->GetMethodID(levelClass, "m_7731_", "(Lnet/minecraft/core/BlockPos;Lnet/minecraft/world/level/block/state/BlockState;I)Z");
    g_env->CallBooleanMethod(levelObj, setBlockMethod, blockPosObj, airState, 3);
}