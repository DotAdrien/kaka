#include "pch.h"
#include "FireSelectSpam.h"

namespace FireSelectSpam {
    bool active = false;

    void Update() {
        if (!active || !g_env) return;

        jclass messageClass = g_env->FindClass("com/tacz/guns/network/message/ClientMessagePlayerFireSelect");
        if (!messageClass) return;

        jmethodID initMessage = g_env->GetMethodID(messageClass, "<init>", "()V");
        if (!initMessage) return;

        jobject messageObj = g_env->NewObject(messageClass, initMessage);
        if (!messageObj) return;

        jclass networkHandlerClass = g_env->FindClass("com/tacz/guns/network/NetworkHandler");
        if (!networkHandlerClass) return;

        jmethodID sendToServer = g_env->GetStaticMethodID(networkHandlerClass, "sendToServer", "(Ljava/lang/Object;)V");
        if (!sendToServer) return;

        for (int i = 0; i < 50; i++) {
            g_env->CallStaticVoidMethod(networkHandlerClass, sendToServer, messageObj);
        }

        g_env->DeleteLocalRef(messageObj);
    }
}