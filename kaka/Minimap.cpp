#include "Minimap.h"
#include "Globals.h"
#include "Mappings.h"
#include "Aimbot.h"
#include "SpeedHack.h"
#include <algorithm>

void UpdateRadarData() {
    if (!g_env || !g_vm) return;
    if (!cacheInit) InitJNICache();
    if (!cacheInit || !getInstance) return;

    jobject mc = g_env->CallStaticObjectMethod(mcClass, getInstance);
    if (!mc) return;

    static ULONGLONG lastTabCheck = 0;
    ULONGLONG now = GetTickCount64();

    if (now - lastTabCheck >= 5000 && getConnectionMethod && playerInfoMapField && getGameModeMethod && getProfileMethod && getProfileNameMethod && getEnumNameMethod) {
        lastTabCheck = now;
        std::vector<std::string> tempSpecs;
        jobject connection = g_env->CallObjectMethod(mc, getConnectionMethod);

        if (connection) {
            jobject playerMap = g_env->GetObjectField(connection, playerInfoMapField);
            if (playerMap) {
                jobject playerList = g_env->CallObjectMethod(playerMap, mapValuesMethod);
                if (playerList) {
                    jobject iterator = g_env->CallObjectMethod(playerList, iteratorMethod);
                    if (iterator) {
                        while (g_env->CallBooleanMethod(iterator, hasNextMethod)) {
                            jobject playerInfo = g_env->CallObjectMethod(iterator, nextMethod);
                            if (playerInfo) {
                                jobject gameMode = g_env->CallObjectMethod(playerInfo, getGameModeMethod);
                                if (gameMode) {
                                    jstring modeNameStr = (jstring)g_env->CallObjectMethod(gameMode, getEnumNameMethod);
                                    if (modeNameStr) {
                                        const char* modeChars = g_env->GetStringUTFChars(modeNameStr, nullptr);
                                        std::string mode(modeChars);
                                        g_env->ReleaseStringUTFChars(modeNameStr, modeChars);
                                        g_env->DeleteLocalRef(modeNameStr);

                                        if (mode == "SPECTATOR") {
                                            jobject profile = g_env->CallObjectMethod(playerInfo, getProfileMethod);
                                            if (profile) {
                                                jstring nameStr = (jstring)g_env->CallObjectMethod(profile, getProfileNameMethod);
                                                if (nameStr) {
                                                    const char* nameChars = g_env->GetStringUTFChars(nameStr, nullptr);
                                                    tempSpecs.push_back(std::string(nameChars));
                                                    g_env->ReleaseStringUTFChars(nameStr, nameChars);
                                                    g_env->DeleteLocalRef(nameStr);
                                                }
                                                g_env->DeleteLocalRef(profile);
                                            }
                                        }
                                    }
                                    g_env->DeleteLocalRef(gameMode);
                                }
                                g_env->DeleteLocalRef(playerInfo);
                            }
                        }
                        g_env->DeleteLocalRef(iterator);
                    }
                    g_env->DeleteLocalRef(playerList);
                }
                g_env->DeleteLocalRef(playerMap);
            }
            g_env->DeleteLocalRef(connection);
        }

        std::lock_guard<std::mutex> lock(g_radar.mtx);
        g_radar.tabSpectators = tempSpecs;
    }

    jobject level = g_env->GetObjectField(mc, levelField);
    jobject myPlayer = g_env->GetObjectField(mc, myPlayerField);

    if (level && myPlayer && getEntitiesMethod && getX && getY && getZ && getYaw) {
        double pX = g_env->CallDoubleMethod(myPlayer, getX);
        double pY = g_env->CallDoubleMethod(myPlayer, getY);
        double pZ = g_env->CallDoubleMethod(myPlayer, getZ);
        float pYaw = g_env->CallFloatMethod(myPlayer, getYaw);

        jobject iterable = g_env->CallObjectMethod(level, getEntitiesMethod);
        std::vector<EntityData> tempEnemies;
        std::vector<EntityData> tempItems;
        std::vector<EntityData> tempOthers;

        double closestDist = 999999.0;
        double targetX = 0, targetY = 0, targetZ = 0;
        bool foundTarget = false;

        if (iterable) {
            jobject iterator = g_env->CallObjectMethod(iterable, iteratorMethod);
            if (iterator) {
                while (g_env->CallBooleanMethod(iterator, hasNextMethod)) {
                    jobject entityObj = g_env->CallObjectMethod(iterator, nextMethod);
                    if (entityObj) {
                        if (!g_env->IsSameObject(entityObj, myPlayer)) {
                            double eX = g_env->CallDoubleMethod(entityObj, getX);
                            double eY = g_env->CallDoubleMethod(entityObj, getY);
                            double eZ = g_env->CallDoubleMethod(entityObj, getZ);
                            float eYaw = g_env->CallFloatMethod(entityObj, getYaw);
                            std::string eName = GetEntityName(entityObj);

                            if (g_env->IsInstanceOf(entityObj, playerClass)) {
                                tempEnemies.push_back({ eX, eY, eZ, eYaw, eName });
                                double dist = sqrt(pow(eX - pX, 2) + pow(eY - pY, 2) + pow(eZ - pZ, 2));
                                if (dist < closestDist) {
                                    closestDist = dist; targetX = eX; targetY = eY + 1.62; targetZ = eZ; foundTarget = true;
                                }
                            }
                            else if (g_env->IsInstanceOf(entityObj, itemEntityClass)) tempItems.push_back({ eX, eY, eZ, eYaw, eName });
                            else if (g_env->IsInstanceOf(entityObj, armorStandClass)) tempOthers.push_back({ eX, eY, eZ, eYaw, eName });
                        }
                        g_env->DeleteLocalRef(entityObj);
                    }
                }
                g_env->DeleteLocalRef(iterator);
            }
            g_env->DeleteLocalRef(iterable);
        }

        if (foundTarget) RunAimbot(pX, pY, pZ, targetX, targetY, targetZ, myPlayer);
        RunSpeedHack(myPlayer, pYaw);

        {
            std::lock_guard<std::mutex> lock(g_radar.mtx);
            g_radar.myX = pX; g_radar.myY = pY; g_radar.myZ = pZ; g_radar.myYaw = pYaw;
            g_radar.enemies = tempEnemies;
            g_radar.items = tempItems;
            g_radar.others = tempOthers;
        }
    }

    if (myPlayer) g_env->DeleteLocalRef(myPlayer);
    if (level) g_env->DeleteLocalRef(level);
    g_env->DeleteLocalRef(mc);
}