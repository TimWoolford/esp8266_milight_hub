#ifndef FUT006_PACKET_FORMATTER_H
#define FUT006_PACKET_FORMATTER_H

#include <PacketFormatter.h>

#define FUT006_COMMAND_INDEX 4
#define FUT006_INTERVALS 10

enum MiLightFUT0006Command: uint8_t {
  FUT006_ALL_ON            = 0x05,
  FUT006_ALL_OFF           = 0x09,
  FUT006_GROUP_1_ON        = 0x08,
  FUT006_GROUP_1_OFF       = 0x0B,
  FUT006_GROUP_2_ON        = 0x0D,
  FUT006_GROUP_2_OFF       = 0x03,
  FUT006_GROUP_3_ON        = 0x07,
  FUT006_GROUP_3_OFF       = 0x0A,
  FUT006_GROUP_4_ON        = 0x02,
  FUT006_GROUP_4_OFF       = 0x06,
  FUT006_BRIGHTNESS_DOWN   = 0x04,
  FUT006_BRIGHTNESS_UP     = 0x0C,
};

class FUT006PacketFormatter : public PacketFormatter {
public:
  FUT006PacketFormatter()
    : PacketFormatter(REMOTE_TYPE_FUT006, 7, 20)
  { }

  virtual bool canHandle(const uint8_t* packet, const size_t len);

  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void command(uint8_t command, uint8_t arg);

  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void enableNightMode();

  virtual void format(uint8_t const* packet, char* buffer);
  virtual void initializePacket(uint8_t* packet);
  virtual void finalizePacket(uint8_t* packet);
  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);

private:
  static uint8_t getStatusButton(uint8_t groupId, MiLightStatus status);
  static uint8_t commandIdToGroup(uint8_t command);
  static MiLightStatus commandToStatus(uint8_t command);
};

#endif
