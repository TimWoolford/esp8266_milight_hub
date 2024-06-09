#include <MiLightRemoteType.h>
#include <Arduino.h>

static const char* REMOTE_NAME_RGBW    = "rgbw";
static const char* REMOTE_NAME_CCT     = "cct";
static const char* REMOTE_NAME_RGB_CCT = "rgb_cct";
static const char* REMOTE_NAME_FUT089  = "fut089";
static const char* REMOTE_NAME_RGB     = "rgb";
static const char* REMOTE_NAME_FUT091  = "fut091";
static const char* REMOTE_NAME_FUT020  = "fut020";
static const char* REMOTE_NAME_FUT006  = "fut006";

MiLightRemoteType MiLightRemoteTypeHelpers::remoteTypeFromString(const String& type) {
  if (type.equalsIgnoreCase(REMOTE_NAME_RGBW) || type.equalsIgnoreCase("fut096")) {
    return REMOTE_TYPE_RGBW;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_CCT) || type.equalsIgnoreCase("fut007")) {
    return REMOTE_TYPE_CCT;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_RGB_CCT) || type.equalsIgnoreCase("fut092")) {
    return REMOTE_TYPE_RGB_CCT;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_FUT089)) {
    return REMOTE_TYPE_FUT089;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_RGB) || type.equalsIgnoreCase("fut098")) {
    return REMOTE_TYPE_RGB;
  }

  if (type.equalsIgnoreCase("v2_cct") || type.equalsIgnoreCase(REMOTE_NAME_FUT091)) {
    return REMOTE_TYPE_FUT091;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_FUT020)) {
    return REMOTE_TYPE_FUT020;
  }

  if (type.equalsIgnoreCase(REMOTE_NAME_FUT006)) {
    return REMOTE_TYPE_FUT006;
  }

  Serial.print(F("remoteTypeFromString: ERROR - tried to fetch remote config for type: "));
  Serial.println(type);

  return REMOTE_TYPE_UNKNOWN;
}

String MiLightRemoteTypeHelpers::remoteTypeToString(const MiLightRemoteType type) {
  switch (type) {
  case REMOTE_TYPE_RGBW:
    return REMOTE_NAME_RGBW;
  case REMOTE_TYPE_CCT:
    return REMOTE_NAME_CCT;
  case REMOTE_TYPE_RGB_CCT:
    return REMOTE_NAME_RGB_CCT;
  case REMOTE_TYPE_FUT089:
    return REMOTE_NAME_FUT089;
  case REMOTE_TYPE_RGB:
    return REMOTE_NAME_RGB;
  case REMOTE_TYPE_FUT091:
    return REMOTE_NAME_FUT091;
  case REMOTE_TYPE_FUT020:
    return REMOTE_NAME_FUT020;
  case REMOTE_TYPE_FUT006:
    return REMOTE_NAME_FUT006;
  default:
    Serial.print(F("remoteTypeToString: ERROR - tried to fetch remote config name for unknown type: "));
    Serial.println(type);
    return "unknown";
  }
}

bool MiLightRemoteTypeHelpers::supportsRgb(const MiLightRemoteType type) {
  switch (type) {
  case REMOTE_TYPE_FUT089:
  case REMOTE_TYPE_RGB:
  case REMOTE_TYPE_RGB_CCT:
  case REMOTE_TYPE_RGBW:
    return true;
  default:
    return false;
  }
}

bool MiLightRemoteTypeHelpers::supportsColorTemp(const MiLightRemoteType type) {
  switch (type) {
  case REMOTE_TYPE_CCT:
  case REMOTE_TYPE_FUT089:
  case REMOTE_TYPE_FUT091:
  case REMOTE_TYPE_RGB_CCT:
    return true;
  default:
    return false;
  }
}

bool MiLightRemoteTypeHelpers::supportsWhiteMode(const MiLightRemoteType type) {
  // These bulbs support switching between rgb/white, and have a "white_mode" command
  switch (type) {
  case REMOTE_TYPE_FUT089:
  case REMOTE_TYPE_RGB_CCT:
  case REMOTE_TYPE_RGBW:
    return true;
  default:
    return false;
  }
}

bool MiLightRemoteTypeHelpers::supportsNumberedEffects(const MiLightRemoteType type) {
  // All bulbs except CCT have 9 modes.  FUT029 and RGB/FUT096 has 9 modes, but they
  // are not selectable directly.  There are only "next mode" commands.
  switch (type) {
  case REMOTE_TYPE_CCT:
  case REMOTE_TYPE_RGB:
  case REMOTE_TYPE_RGB_CCT:
  case REMOTE_TYPE_FUT006:
  case REMOTE_TYPE_FUT020:
    return false;
  default:
    return true;
  }
}
