#include "FireSelectSpam.h"

namespace FireSelectSpam {
    bool active = false;

    void Update() {
        if (!active || !g_env) return;

        // Trouve le paquet du mod TACZ 🤓
        jclass messageClass = g_env->FindClass("com/tacz/guns/network/message/ClientMessagePlayerFireSelect");
        jmethodID initMessage = g_env->GetMethodID(messageClass, "<init>", "()V");
        jobject messageObj = g_env->NewObject(messageClass, initMessage);

        // Trouve le NetworkHandler du mod 🫣
        jclass networkHandlerClass = g_env->FindClass("com/tacz/guns/network/NetworkHandler");
        jmethodID sendToServer = g_env->GetStaticMethodID(networkHandlerClass, "sendToServer", "(Ljava/lang/Object;)V");

        // On spamme 50 fois par tick 🤣
        for (int i = 0; i < 50; i++) {
            g_env->CallStaticVoidMethod(networkHandlerClass, sendToServer, messageObj);
        }

        g_env->DeleteLocalRef(messageObj);
    }
}