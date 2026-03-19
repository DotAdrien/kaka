#pragma once
#include "Globals.h"

extern jclass mcClass, clientLevelClass, playerClass, itemEntityClass, armorStandClass, componentClass;
extern jclass connectionClass, playerInfoClass, gameProfileClass, gameTypeClass, vec3Class;
extern jclass blockHitResultClass, blockClass, blocksClass, blockPosClass, vec3iClass;

extern jmethodID getInstance, getX, getY, getZ, getYaw;
extern jmethodID getEntitiesMethod, iteratorMethod, hasNextMethod, nextMethod;
extern jmethodID getNameMethod, getStringMethod;
extern jmethodID setYRotMethod, setXRotMethod;
extern jmethodID getConnectionMethod, getProfileMethod, getGameModeMethod;
extern jmethodID getProfileNameMethod, getEnumNameMethod, mapValuesMethod;

extern jmethodID getDeltaMovementMethod, setDeltaMovementMethod;
extern jmethodID getBlockPosMethod, getBlockStateMethod, defaultStateMethod, setBlockMethod;
extern jmethodID getBlockX, getBlockY, getBlockZ;

extern jfieldID vec3X, vec3Y, vec3Z;
extern jfieldID levelField, myPlayerField, playerInfoMapField;
extern jfieldID hitResultField, airField;

void InitJNICache();
std::string GetEntityName(jobject entityObj);