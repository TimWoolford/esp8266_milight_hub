#pragma once

#include <list>
#include <map>
#include <BulbId.h>
#include <MqttClient.h>


namespace HomeAssistantValues
{
  // Color Modes
  static const char BRIGHTNESS[] = "brightness";
  static const char RGB[] = "rgb";
  static const char COLOR_TEMP[] = "color_temp";

  // Effects
  static const char NORMAL[] = "normal";
  static const char NIGHT_MODE[] = "night_mode";
  static const char WHITE_MODE[] = "white_mode";
}

class HomeAssistantDiscoveryClient {
public:
  HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient);

  void addConfig(const char* alias, const BulbId& bulbId);
  void removeConfig(const BulbId& bulbId);

  void sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases);
  void removeOldDevices(const std::map<uint32_t, BulbId>& aliases);

private:
  Settings& settings;
  MqttClient* mqttClient;

  String buildTopic(const BulbId& bulbId) const;
  static String bindTopicVariables(const String& topic, const char* alias, const BulbId& bulbId);
  static void addNumberedEffects(const JsonArray& effectsList, const std::list<uint8_t>& numbers);
};
