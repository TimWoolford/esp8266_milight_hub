#include <HomeAssistantDiscoveryClient.h>
#include <Units.h>
#include <ESP8266WiFi.h>

HomeAssistantDiscoveryClient::HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient)
  : settings(settings)
    , mqttClient(mqttClient) {}

void HomeAssistantDiscoveryClient::sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: Sending %d discoverable devices...\n"), aliases.size());
#endif

  for (const auto& alias : aliases) {
    addConfig(alias.first.c_str(), alias.second.bulbId);
  }
}

void HomeAssistantDiscoveryClient::removeOldDevices(const std::map<uint32_t, BulbId>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: Removing %d discoverable devices...\n"), aliases.size());
#endif

  for (const auto& alias : aliases) {
    removeConfig(alias.second);
  }
}

void HomeAssistantDiscoveryClient::removeConfig(const BulbId& bulbId) {
  // Remove by publishing an empty message
  String topic = buildTopic(bulbId);
  mqttClient->send(topic.c_str(), "", true);
}

void HomeAssistantDiscoveryClient::addConfig(const char* alias, const BulbId& bulbId) {
  String topic = buildTopic(bulbId);
  DynamicJsonDocument config(1024);

  // Unique ID for this device + alias combo
  char uniqueId[30];
  snprintf_P(uniqueId, sizeof(uniqueId), PSTR("%X-%s"), EspClass::getChipId(), alias);

  // String to ID the firmware version
  char fwVersion[100];
  snprintf_P(fwVersion, sizeof(fwVersion), PSTR("esp8266_milight_hub v%s"), QUOTE(MILIGHT_HUB_VERSION));

  // URL to the device
  char configurationUrl[23];
  snprintf_P(configurationUrl, sizeof(configurationUrl), PSTR("http://%s"), WiFi.localIP().toString().c_str());

  config[F("dev_cla")] = F("light");
  config[F("schema")] = F("json");
  config[F("name")] = alias;
  config[F("cmd_t")] = mqttClient->bindTopicString(settings.mqttTopicPattern, bulbId);
  config[F("stat_t")] = mqttClient->bindTopicString(settings.mqttStateTopicPattern, bulbId);
  config[F("uniq_id")] = uniqueId;

  JsonObject deviceMetadata = config.createNestedObject(F("dev"));
  deviceMetadata[F("name")] = settings.hostname;
  deviceMetadata[F("sw")] = fwVersion;
  deviceMetadata[F("mf")] = F("espressif");
  deviceMetadata[F("mdl")] = QUOTE(FIRMWARE_VARIANT);
  deviceMetadata[F("ids")] = String(EspClass::getChipId());
  deviceMetadata[F("cu")] = configurationUrl;

  // HomeAssistant only supports simple client availability
  if (settings.mqttClientStatusTopic.length() > 0 && settings.simpleMqttClientStatus) {
    JsonObject availability = config.createNestedObject(F("avty"));
    availability[F("t")] = settings.mqttClientStatusTopic;
    availability[F("pl_avail")] = F("connected");
    availability[F("pl_not_avail")] = F("disconnected");
  }

  // Configure supported commands based on the bulb type

  // effect_list
  config[GroupStateFieldNames::EFFECT] = true;
  JsonArray effectsList = config.createNestedArray(F("fx_list"));
  effectsList.add(HomeAssistantValues::NORMAL);
  effectsList.add(HomeAssistantValues::NIGHT_MODE);

  if (MiLightRemoteTypeHelpers::supportsWhiteMode(bulbId.deviceType)) {
    effectsList.add(HomeAssistantValues::WHITE_MODE);
  }

  if (MiLightRemoteTypeHelpers::supportsNumberedEffects(bulbId.deviceType)) {
    addNumberedEffects(effectsList, {0, 1, 2, 3, 4, 5, 6, 7, 8});
  }

  // supported color modes
  JsonArray supportedColorModes = config.createNestedArray(F("sup_clrm"));

  if (MiLightRemoteTypeHelpers::supportsRgb(bulbId.deviceType)) {
    supportedColorModes.add(HomeAssistantValues::RGB);
  }

  if (MiLightRemoteTypeHelpers::supportsColorTemp(bulbId.deviceType)) {
    supportedColorModes.add(HomeAssistantValues::COLOR_TEMP);
    config[F("max_mirs")] = COLOR_TEMP_MAX_MIREDS;
    config[F("min_mirs")] = COLOR_TEMP_MIN_MIREDS;
  }

  if (supportedColorModes.size() == 0) {
    supportedColorModes.add(HomeAssistantValues::BRIGHTNESS);
  } else {
    config[GroupStateFieldNames::BRIGHTNESS] = true;
  }

  String message;
  serializeJson(config, message);

#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: adding discoverable device: %s...\n"), alias);
  Serial.printf_P(PSTR("  topic: %s\nconfig: %s\n"), topic.c_str(), message.c_str());
#endif

  mqttClient->send(topic.c_str(), message.c_str(), true);
}

// Topic syntax:
//   <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
//
// source: https://www.home-assistant.io/docs/mqtt/discovery/
String HomeAssistantDiscoveryClient::buildTopic(const BulbId& bulbId) const {
  String topic = settings.homeAssistantDiscoveryPrefix;

  // Don't require the user to entier a "/" (or break things if they do)
  if (!topic.endsWith("/")) {
    topic += "/";
  }

  topic += "light/";
  // Use a static ID that doesn't depend on configuration.
  topic += "milight_hub_" + String(EspClass::getChipId());

  // make the object ID based on the actual parameters rather than the alias.
  topic += "/";
  topic += MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType);
  topic += "_";
  topic += bulbId.getHexDeviceId();
  topic += "_";
  topic += bulbId.groupId;
  topic += "/config";

  return topic;
}

String HomeAssistantDiscoveryClient::bindTopicVariables(const String& topic, const char* alias, const BulbId& bulbId) {
  String boundTopic = topic;
  String hexDeviceId = bulbId.getHexDeviceId();

  boundTopic.replace(":device_alias", alias);
  boundTopic.replace(":device_id", hexDeviceId);
  boundTopic.replace(":hex_device_id", hexDeviceId);
  boundTopic.replace(":dec_device_id", String(bulbId.deviceId));
  boundTopic.replace(":device_type", MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType));
  boundTopic.replace(":group_id", String(bulbId.groupId));

  return boundTopic;
}

void HomeAssistantDiscoveryClient::addNumberedEffects(const JsonArray& effectsList, const std::list<uint8_t>& numbers) {
  for (const uint8_t& alias : numbers) {
    effectsList.add(String(alias));
  }
}
