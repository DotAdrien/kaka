#include "DupePoC.h"
#include <fstream>
#include <string>

void DupePoC::Run(JNIEnv* env, int gunSlot, int attachSlot) {
    std::ofstream log("poc_detailed_debug.txt", std::ios::app);
    log << "--- [START] Detailed PoC Session ---" << std::endl;
    log << "Paramètres reçus: gunSlot=" << gunSlot << ", attachSlot=" << attachSlot << std::endl;

    if (!env) { log << "CRITICAL: env est NULL" << std::endl; return; }

    // 1. CHERCHER LE CLASSLOADER
    jclass threadClass = env->FindClass("java/lang/Thread");
    jmethodID getAllStackTracesMid = env->GetStaticMethodID(threadClass, "getAllStackTraces", "()Ljava/util/Map;");
    jobject map = env->CallStaticObjectMethod(threadClass, getAllStackTracesMid);

    jclass mapClass = env->GetObjectClass(map);
    jobject set = env->CallObjectMethod(map, env->GetMethodID(mapClass, "keySet", "()Ljava/util/Set;"));

    jclass setClass = env->GetObjectClass(set);
    jobjectArray threads = (jobjectArray)env->CallObjectMethod(set, env->GetMethodID(setClass, "toArray", "()[Ljava/lang/Object;"));

    jobject classLoader = nullptr;
    int threadCount = env->GetArrayLength(threads);

    for (int i = 0; i < threadCount; i++) {
        jobject t = env->GetObjectArrayElement(threads, i);
        jstring nameStr = (jstring)env->CallObjectMethod(t, env->GetMethodID(threadClass, "getName", "()Ljava/lang/String;"));
        const char* nameChars = env->GetStringUTFChars(nameStr, nullptr);
        std::string tName(nameChars);
        env->ReleaseStringUTFChars(nameStr, nameChars);

        if (tName == "Render thread" || tName == "Client thread") {
            classLoader = env->CallObjectMethod(t, env->GetMethodID(threadClass, "getContextClassLoader", "()Ljava/lang/ClassLoader;"));
            log << "INFO: ClassLoader récupéré depuis: " << tName << std::endl;
            break;
        }
    }

    if (!classLoader) { log << "ERROR: Aucun ClassLoader trouvé !" << std::endl; return; }

    // Helper pour charger les classes du mod
    jclass classClass = env->FindClass("java/lang/Class");
    jmethodID forNameMid = env->GetStaticMethodID(classClass, "forName", "(Ljava/lang/String;ZLjava/lang/ClassLoader;)Ljava/lang/Class;");
    auto loadClass = [&](const char* name) -> jclass {
        jstring cn = env->NewStringUTF(name);
        jclass c = (jclass)env->CallStaticObjectMethod(classClass, forNameMid, cn, JNI_TRUE, classLoader);
        env->DeleteLocalRef(cn);
        if (env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }
        return c;
        };

    // 2. VERIFICATION ATTACHMENT TYPE
    jclass attachTypeClass = loadClass("com.tacz.guns.api.item.attachment.AttachmentType");
    if (!attachTypeClass) { log << "ERROR: com.tacz.guns.api.item.attachment.AttachmentType non trouvé" << std::endl; return; }

    jfieldID extMagField = env->GetStaticFieldID(attachTypeClass, "EXTENDED_MAG", "Lcom/tacz/guns/api/item/attachment/AttachmentType;");
    jobject extMagEnum = env->GetStaticObjectField(attachTypeClass, extMagField);

    // Log du nom de l'enum pour être sur
    jmethodID nameMid = env->GetMethodID(attachTypeClass, "name", "()Ljava/lang/String;");
    jstring enumName = (jstring)env->CallObjectMethod(extMagEnum, nameMid);
    const char* enumStr = env->GetStringUTFChars(enumName, nullptr);
    log << "INFO: Utilisation de l'AttachmentType: " << enumStr << std::endl;
    env->ReleaseStringUTFChars(enumName, enumStr);

    // 3. PREPARATION DU PAQUET
    jclass msgClass = loadClass("com.tacz.guns.network.message.ClientMessageRefitGun");
    if (!msgClass) { log << "ERROR: ClientMessageRefitGun non trouvé" << std::endl; return; }

    jmethodID ctor = env->GetMethodID(msgClass, "<init>", "(IILcom/tacz/guns/api/item/attachment/AttachmentType;)V");
    if (!ctor) { log << "ERROR: Constructeur ClientMessageRefitGun(int, int, AttachmentType) introuvable" << std::endl; return; }

    jobject msgObj = env->NewObject(msgClass, ctor, attachSlot, gunSlot, extMagEnum);
    if (!msgObj) { log << "ERROR: Échec NewObject pour le paquet" << std::endl; return; }
    log << "INFO: Paquet créé avec succès" << std::endl;

    // 4. ENVOI VIA NETWORKHANDLER
    jclass netHandlerClass = loadClass("com.tacz.guns.network.NetworkHandler");
    if (!netHandlerClass) { log << "ERROR: NetworkHandler non trouvé" << std::endl; return; }

    // On essaye de trouver le champ CHANNEL (il peut changer selon la version de Forge)
    jfieldID chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/simple/SimpleChannel;");
    if (!chanField) {
        env->ExceptionClear();
        chanField = env->GetStaticFieldID(netHandlerClass, "CHANNEL", "Lnet/minecraftforge/network/SimpleChannel;");
    }

    if (!chanField) { log << "ERROR: Champ static CHANNEL introuvable dans NetworkHandler" << std::endl; return; }

    jobject channel = env->GetStaticObjectField(netHandlerClass, chanField);
    jclass chanClass = env->GetObjectClass(channel);

    // On cherche la méthode d'envoi
    jmethodID sendMid = env->GetMethodID(chanClass, "sendToServer", "(Ljava/lang/Object;)V");
    if (!sendMid) {
        env->ExceptionClear();
        sendMid = env->GetMethodID(chanClass, "m_12345_", "(Ljava/lang/Object;)V"); // Obfuscated
    }

    if (!sendMid) { log << "ERROR: Méthode sendToServer introuvable" << std::endl; return; }

    // ENVOI FINAL
    env->CallVoidMethod(channel, sendMid, msgObj);
    log << "SUCCESS: Paquet envoyé au serveur ! 🚀" << std::endl;
    log << "--- [END] ---" << std::endl;

    // Nettoyage local
    env->DeleteLocalRef(msgObj);
    env->DeleteLocalRef(channel);
    env->DeleteLocalRef(classLoader);
}