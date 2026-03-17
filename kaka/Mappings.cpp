#include "Mappings.h"

// Définitions des variables globales 🤓
RadarData g_radar;
JavaVM* g_vm = nullptr;
JNIEnv* g_env = nullptr;
bool cacheInit = false;

jclass mcClass, clientLevelClass, playerClass, itemEntityClass, armorStandClass, componentClass;
jclass connectionClass, playerInfoClass, gameProfileClass, gameTypeClass;
jmethodID getInstance, getX, getY, getZ, getYaw;
jmethodID getEntitiesMethod, iteratorMethod, hasNextMethod, nextMethod;
jmethodID getNameMethod, getStringMethod;
jmethodID setYRotMethod, setXRotMethod;
jmethodID getConnectionMethod, getProfileMethod, getGameModeMethod;
jmethodID getProfileNameMethod, getEnumNameMethod, mapValuesMethod;
jfieldID levelField, myPlayerField, playerInfoMapField;

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
    }

    if (connectionClass && playerInfoClass && gameProfileClass) {
        getConnectionMethod = g_env->GetMethodID(mcClass, "m_91403_", "()Lnet/minecraft/client/multiplayer/ClientPacketListener;");
        playerInfoMapField = g_env->GetFieldID(connectionClass, "f_104892_", "Ljava/util/Map;");
        getProfileMethod = g_env->GetMethodID(playerInfoClass, "m_105312_", "()Lcom/mojang/authlib/GameProfile;");
        getGameModeMethod = g_env->GetMethodID(playerInfoClass, "m_105315_", "()Lnet/minecraft/world/level/GameType;");
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
    g_env->DeleteLocalRef(entityClass);

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