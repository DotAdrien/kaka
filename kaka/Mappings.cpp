#include "Mappings.h"

// On crée les variables en mémoire ici 🥵
jclass mcClass = nullptr, clientLevelClass = nullptr, playerClass = nullptr, itemEntityClass = nullptr, armorStandClass = nullptr, componentClass = nullptr;
jclass connectionClass = nullptr, playerInfoClass = nullptr, gameProfileClass = nullptr, gameTypeClass = nullptr, vec3Class = nullptr;
jmethodID getInstance = nullptr, getX = nullptr, getY = nullptr, getZ = nullptr, getYaw = nullptr;
jmethodID getEntitiesMethod = nullptr, iteratorMethod = nullptr, hasNextMethod = nullptr, nextMethod = nullptr;
jmethodID getNameMethod = nullptr, getStringMethod = nullptr;
jmethodID setYRotMethod = nullptr, setXRotMethod = nullptr;
jmethodID getConnectionMethod = nullptr, getProfileMethod = nullptr, getGameModeMethod = nullptr;
jmethodID getProfileNameMethod = nullptr, getEnumNameMethod = nullptr, mapValuesMethod = nullptr;
jmethodID getDeltaMovementMethod = nullptr, setDeltaMovementMethod = nullptr;
jfieldID vec3X = nullptr, vec3Y = nullptr, vec3Z = nullptr;
jfieldID levelField = nullptr, myPlayerField = nullptr, playerInfoMapField = nullptr;

jclass GetClass(JNIEnv* env, JavaVM* vm, const char* name) {
    jvmtiEnv* jvmti;
    vm->GetEnv((void**)&jvmti, JVMTI_VERSION_1_2);
    jint count; jclass* classes;
    jvmti->GetLoadedClasses(&count, &classes);
    jclass found = nullptr;
    for (int i = 0; i < count; i++) {
        char* sig;
        jvmti->GetClassSignature(classes[i], &sig, nullptr);
        if (sig && strcmp(sig, name) == 0) {
            found = (jclass)env->NewGlobalRef(classes[i]);
            jvmti->Deallocate((unsigned char*)sig); break;
        }
        if (sig) jvmti->Deallocate((unsigned char*)sig);
    }
    jvmti->Deallocate((unsigned char*)classes);
    return found;
}

void InitJNICache() {
    mcClass = GetClass(g_env, g_vm, "Lnet/minecraft/client/Minecraft;");
    clientLevelClass = GetClass(g_env, g_vm, "Lnet/minecraft/client/multiplayer/ClientLevel;");
    playerClass = GetClass(g_env, g_vm, "Lnet/minecraft/world/entity/player/Player;");
    itemEntityClass = GetClass(g_env, g_vm, "Lnet/minecraft/world/entity/item/ItemEntity;");
    armorStandClass = GetClass(g_env, g_vm, "Lnet/minecraft/world/entity/decoration/ArmorStand;");
    jclass entityClass = GetClass(g_env, g_vm, "Lnet/minecraft/world/entity/Entity;");
    componentClass = GetClass(g_env, g_vm, "Lnet/minecraft/network/chat/Component;");

    connectionClass = GetClass(g_env, g_vm, "Lnet/minecraft/client/multiplayer/ClientPacketListener;");
    playerInfoClass = GetClass(g_env, g_vm, "Lnet/minecraft/client/multiplayer/PlayerInfo;");
    gameProfileClass = GetClass(g_env, g_vm, "Lcom/mojang/authlib/GameProfile;");
    gameTypeClass = GetClass(g_env, g_vm, "Lnet/minecraft/world/level/GameType;");
    vec3Class = GetClass(g_env, g_vm, "Lnet/minecraft/world/phys/Vec3;");

    jclass localIterable = g_env->FindClass("java/lang/Iterable");
    jclass localIterator = g_env->FindClass("java/util/Iterator");
    jclass localEnum = g_env->FindClass("java/lang/Enum");
    jclass localMap = g_env->FindClass("java/util/Map");

    if (mcClass && entityClass && clientLevelClass && playerClass) {
        getInstance = g_env->GetStaticMethodID(mcClass, "m_91087_", "()Lnet/minecraft/client/Minecraft;");
        levelField = g_env->GetFieldID(mcClass, "f_91073_", "Lnet/minecraft/client/multiplayer/ClientLevel;");
        myPlayerField = g_env->GetFieldID(mcClass, "f_91074_", "Lnet/minecraft/client/player/LocalPlayer;");

        getX = g_env->GetMethodID(entityClass, "m_20185_", "()D");
        getY = g_env->GetMethodID(entityClass, "m_20186_", "()D");
        getZ = g_env->GetMethodID(entityClass, "m_20189_", "()D");
        getYaw = g_env->GetMethodID(entityClass, "m_146908_", "()F");

        setYRotMethod = g_env->GetMethodID(entityClass, "m_146922_", "(F)V");
        setXRotMethod = g_env->GetMethodID(entityClass, "m_146926_", "(F)V");

        getNameMethod = g_env->GetMethodID(entityClass, "m_7755_", "()Lnet/minecraft/network/chat/Component;");
        getStringMethod = g_env->GetMethodID(componentClass, "getString", "()Ljava/lang/String;");
        getEntitiesMethod = g_env->GetMethodID(clientLevelClass, "m_104735_", "()Ljava/lang/Iterable;");

        // SpeedHack ! 🚀
        getDeltaMovementMethod = g_env->GetMethodID(entityClass, "m_20184_", "()Lnet/minecraft/world/phys/Vec3;");
        setDeltaMovementMethod = g_env->GetMethodID(entityClass, "m_20334_", "(DDD)V");
    }

    if (vec3Class) {
        vec3X = g_env->GetFieldID(vec3Class, "f_82479_", "D");
        if (!vec3X) { g_env->ExceptionClear(); vec3X = g_env->GetFieldID(vec3Class, "x", "D"); }
        vec3Y = g_env->GetFieldID(vec3Class, "f_82480_", "D");
        if (!vec3Y) { g_env->ExceptionClear(); vec3Y = g_env->GetFieldID(vec3Class, "y", "D"); }
        vec3Z = g_env->GetFieldID(vec3Class, "f_82481_", "D");
        if (!vec3Z) { g_env->ExceptionClear(); vec3Z = g_env->GetFieldID(vec3Class, "z", "D"); }
    }

    if (connectionClass && playerInfoClass && gameProfileClass) {
        getConnectionMethod = g_env->GetMethodID(mcClass, "m_91403_", "()Lnet/minecraft/client/multiplayer/ClientPacketListener;");
        playerInfoMapField = g_env->GetFieldID(connectionClass, "f_104892_", "Ljava/util/Map;");
        getProfileMethod = g_env->GetMethodID(playerInfoClass, "m_105312_", "()Lcom/mojang/authlib/GameProfile;");
        getGameModeMethod = g_env->GetMethodID(playerInfoClass, "m_105325_", "()Lnet/minecraft/world/level/GameType;");
        getProfileNameMethod = g_env->GetMethodID(gameProfileClass, "getName", "()Ljava/lang/String;");
    }

    getEnumNameMethod = g_env->GetMethodID(localEnum, "name", "()Ljava/lang/String;");
    mapValuesMethod = g_env->GetMethodID(localMap, "values", "()Ljava/util/Collection;");
    iteratorMethod = g_env->GetMethodID(localIterable, "iterator", "()Ljava/util/Iterator;");
    hasNextMethod = g_env->GetMethodID(localIterator, "hasNext", "()Z");
    nextMethod = g_env->GetMethodID(localIterator, "next", "()Ljava/lang/Object;");

    g_env->DeleteLocalRef(localIterable);
    g_env->DeleteLocalRef(localIterator);
    g_env->DeleteLocalRef(localEnum);
    g_env->DeleteLocalRef(localMap);
    g_env->DeleteGlobalRef(entityClass);

    cacheInit = true;
}

std::string GetEntityName(jobject entityObj) {
    if (!getNameMethod) return "Inconnu";
    jobject component = g_env->CallObjectMethod(entityObj, getNameMethod);
    if (!component) return "Inconnu";
    jstring jstr = (jstring)g_env->CallObjectMethod(component, getStringMethod);
    if (!jstr) { g_env->DeleteLocalRef(component); return "Inconnu"; }
    const char* chars = g_env->GetStringUTFChars(jstr, nullptr);
    std::string name(chars);
    g_env->ReleaseStringUTFChars(jstr, chars);
    g_env->DeleteLocalRef(jstr);
    g_env->DeleteLocalRef(component);
    return name;
}