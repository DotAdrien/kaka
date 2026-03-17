#pragma once
#include "Globals.h"

extern jclass mcClass, clientLevelClass, playerClass, itemEntityClass, armorStandClass, componentClass;
extern jclass connectionClass, playerInfoClass, gameProfileClass, gameTypeClass;
extern jmethodID getInstance, getX, getY, getZ, getYaw;
extern jmethodID getEntitiesMethod, iteratorMethod, hasNextMethod, nextMethod;
extern jmethodID getNameMethod, getStringMethod;
extern jmethodID setYRotMethod, setXRotMethod;
extern jmethodID getConnectionMethod, getProfileMethod, getGameModeMethod;
extern jmethodID getProfileNameMethod, getEnumNameMethod, mapValuesMethod;
extern jfieldID levelField, myPlayerField, playerInfoMapField;

void InitJNICache();
std::string GetEntityName(jobject entityObj);