#include "DupePoC.h"
#include <string>
#include <fstream>



void DupePoC::Run(JNIEnv* env, int gunSlot, int attachSlot) {

    if (!env) return;


    jclass threadClass = env->FindClass("java/lang/Thread");
    if (!threadClass) { env->ExceptionClear(); return; }

    jmethodID getAllStackTracesMid = env->GetStaticMethodID(threadClass, "getAllStackTraces", "()Ljava/util/Map;");
    if (!getAllStackTracesMid) return;


    jobject map = env->CallStaticObjectMethod(threadClass, getAllStackTracesMid);
    if (!map) return;


    jclass mapClass = env->GetObjectClass(map);
    if (!mapClass) return;


    jmethodID keySetMid = env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;");
    if (!keySetMid) return;


    jobject set = env->CallObjectMethod(map, keySetMid);
    if (!set) return;


    jclass setClass = env->GetObjectClass(set);
    if (!setClass) return;


    jmethodID toArrayMid = env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;");
    if (!toArrayMid) return;


    jobjectArray threads = (jobjectArray)env->CallObjectMethod(set, toArrayMid);
    if (!threads) return;


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

            classLoader = env->CallObjectMethod(t, getContextClassLoaderMid);
            env->DeleteLocalRef(t);
            break;
        }
        env->DeleteLocalRef(t);
    }

    if (!classLoader) return;


    jclass classClass = env->FindClass("java/lang/Class");
    if (!classClass) { env->ExceptionClear(); return; }


    jmethodID forNameMid = env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
    if (!forNameMid) return;

    auto loadClass = [&](const char* name) -> jclass {
        jstring cn = env->NewStringUTF(name);
        jclass c = (jclass)env->CallStaticObjectMethod(classClass, forNameMid, cn, JNI_TRUE, classLoader);
        env->DeleteLocalRef(cn);
        if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
        return c;
        };


    jclass attachTypeClass = loadClass("com.tacz.guns.api.item.attachment.AttachmentType");
    if (!attachTypeClass) return;


    jfieldID extMagField = env->GetStaticFieldID(attachTypeClass, "EXTENDED_MAG", "Lcom/tacz/guns/api/item/attachment/AttachmentType;");
    if (!extMagField) { env->ExceptionClear(); return; }


    jobject extMagEnum = env->GetStaticObjectField(attachTypeClass, extMagField);
    if (!extMagEnum) return;

    jclass msgClass = loadClass("com.tacz.guns.network.message.ClientMessageRefitGun");
    if (!msgClass) return;

    jmethodID ctor = env->GetMethodID(msgClass, "<init>", "(IILcom/tacz/guns/api/item/attachment/AttachmentType;)V");
    if (!ctor) { env->ExceptionClear(); return; }


    jobject msgObj = env->NewObject(msgClass, ctor, attachSlot, gunSlot, extMagEnum);
    if (!msgObj) return;

    jclass netHandlerClass = loadClass("com.tacz.guns.network.NetworkHandler");
    if (!netHandlerClass) return;

    jfieldID chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/simple/SimpleChannel;");
    if (!chanField) {
        env->ExceptionClear();
        chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/SimpleChannel;");
    }
    if (!chanField) { env->ExceptionClear(); return; }


    jobject channel = env->GetStaticObjectField(netHandlerClass, chanField);
    if (!channel) return;


    jclass chanClass = env->GetObjectClass(channel);
    if (!chanClass) return;


    jmethodID sendMid = env->GetMethodID(chanClass, "sendToServer", "(Ljava/lang/Object;)V");
    if (!sendMid) {
        env->ExceptionClear();
        sendMid = env->GetMethodID(chanClass, "m_12345_", "(Ljava/lang/Object;)V");
    }
    if (!sendMid) return;


    env->CallVoidMethod(channel, sendMid, msgObj);


    env->DeleteLocalRef(msgObj);
    env->DeleteLocalRef(channel);
    env->DeleteLocalRef(classLoader);

}