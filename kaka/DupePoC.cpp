#include "DupePoC.h"
#include <string>
#include <fstream>

void Step(const char* msg) {
    std::ofstream log("kaka_step_log.txt", std::ios::app);
    log << "Etape: " << msg << "\n";
}

void DupePoC::Run(JNIEnv* env, int gunSlot, int attachSlot) {
    Step("Init");
    if (!env) return;

    Step("FindClass Thread");
    jclass threadClass = env->FindClass("java/lang/Thread");
    if (!threadClass) { env->ExceptionClear(); return; }

    Step("Get getAllStackTraces");
    jmethodID getAllStackTracesMid = env->GetStaticMethodID(threadClass, "getAllStackTraces", "()Ljava/util/Map;");
    if (!getAllStackTracesMid) return;

    Step("Call getAllStackTraces");
    jobject map = env->CallStaticObjectMethod(threadClass, getAllStackTracesMid);
    if (!map) return;

    Step("Get Map Class");
    jclass mapClass = env->GetObjectClass(map);
    if (!mapClass) return;

    Step("Get keySet Method");
    jmethodID keySetMid = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
    if (!keySetMid) return;

    Step("Call keySet");
    jobject set = env->CallObjectMethod(map, keySetMid);
    if (!set) return;

    Step("Get Set Class");
    jclass setClass = env->GetObjectClass(set);
    if (!setClass) return;

    Step("Get toArray Method");
    jmethodID toArrayMid = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
    if (!toArrayMid) return;

    Step("Call toArray");
    jobjectArray threads = (jobjectArray)env->CallObjectMethod(set, toArrayMid);
    if (!threads) return;

    Step("Loop threads");
    jobject classLoader = nullptr;
    int threadCount = env->GetArrayLength(threads);

    jmethodID getNameMid = env->GetMethodID(threadClass, "getName", "()Ljava/lang/String;");
    jmethodID getContextClassLoaderMid = env->GetMethodID(threadClass, "getContextClassLoader", "()Ljava/lang/ClassLoader;");

    for (int i = 0; i < threadCount; i++) {
        jobject t = env->GetObjectArrayElement(threads, i);
        if (!t) continue;

        jstring nameStr = (jstring)env->CallObjectMethod(t, getNameMid);
        if (!nameStr) { env->DeleteLocalRef(t); continue; }

        const char* nameChars = env->GetStringUTFChars(nameStr, nullptr);
        if (!nameChars) { env->DeleteLocalRef(nameStr); env->DeleteLocalRef(t); continue; }

        std::string tName(nameChars);
        env->ReleaseStringUTFChars(nameStr, nameChars);
        env->DeleteLocalRef(nameStr);

        if (tName == "Render thread" || tName == "Client thread") {
            Step("Found thread");
            classLoader = env->CallObjectMethod(t, getContextClassLoaderMid);
            env->DeleteLocalRef(t);
            break;
        }
        env->DeleteLocalRef(t);
    }

    if (!classLoader) return;

    Step("Find Class java.lang.Class");
    jclass classClass = env->FindClass("java/lang/Class");
    if (!classClass) { env->ExceptionClear(); return; }

    Step("Get forName method");
    jmethodID forNameMid = env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
    if (!forNameMid) return;

    auto loadClass = [&](const char* name) -> jclass {
        jstring cn = env->NewStringUTF(name);
        jclass c = (jclass)env->CallStaticObjectMethod(classClass, forNameMid, cn, JNI_TRUE, classLoader);
        env->DeleteLocalRef(cn);
        if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
        return c;
        };

    Step("Load AttachmentType");
    jclass attachTypeClass = loadClass("com.tacz.guns.api.item.attachment.AttachmentType");
    if (!attachTypeClass) return;

    Step("Get EXTENDED_MAG");
    jfieldID extMagField = env->GetStaticFieldID(attachTypeClass, "EXTENDED_MAG", "Lcom/tacz/guns/api/item/attachment/AttachmentType;");
    if (!extMagField) { env->ExceptionClear(); return; }

    Step("Get EXTENDED_MAG object");
    jobject extMagEnum = env->GetStaticObjectField(attachTypeClass, extMagField);
    if (!extMagEnum) return;

    Step("Load ClientMessageRefitGun");
    jclass msgClass = loadClass("com.tacz.guns.network.message.ClientMessageRefitGun");
    if (!msgClass) return;

    Step("Get ctor");
    jmethodID ctor = env->GetMethodID(msgClass, "<init>", "(IILcom/tacz/guns/api/item/attachment/AttachmentType;)V");
    if (!ctor) { env->ExceptionClear(); return; }

    Step("NewObject msgObj");
    jobject msgObj = env->NewObject(msgClass, ctor, attachSlot, gunSlot, extMagEnum);
    if (!msgObj) return;

    Step("Load NetworkHandler");
    jclass netHandlerClass = loadClass("com.tacz.guns.network.NetworkHandler");
    if (!netHandlerClass) return;

    Step("Get CHANNEL field");
    jfieldID chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/simple/SimpleChannel;");
    if (!chanField) {
        env->ExceptionClear();
        chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/SimpleChannel;");
    }
    if (!chanField) { env->ExceptionClear(); return; }

    Step("Get CHANNEL object");
    jobject channel = env->GetStaticObjectField(netHandlerClass, chanField);
    if (!channel) return;

    Step("Get Channel Class");
    jclass chanClass = env->GetObjectClass(channel);
    if (!chanClass) return;

    Step("Get sendToServer method");
    jmethodID sendMid = env->GetMethodID(chanClass, "sendToServer", "(Ljava/lang/Object;)V");
    if (!sendMid) {
        env->ExceptionClear();
        sendMid = env->GetMethodID(chanClass, "m_12345_", "(Ljava/lang/Object;)V");
    }
    if (!sendMid) return;

    Step("CallVoidMethod");
    env->CallVoidMethod(channel, sendMid, msgObj);

    Step("Cleanup");
    env->DeleteLocalRef(msgObj);
    env->DeleteLocalRef(channel);
    env->DeleteLocalRef(classLoader);
    Step("Termine");
}