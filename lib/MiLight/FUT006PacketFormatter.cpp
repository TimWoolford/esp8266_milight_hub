#include <FUT006PacketFormatter.h>
#include <MiLightCommands.h>

static constexpr uint8_t PROTOCOL_ID = 0x5A;

bool FUT006PacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && packet[0] == PROTOCOL_ID;
}

void FUT006PacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Byte 0: Packet length = 7 bytes

  // Byte 1: CCT protocol
  packet[packetPtr++] = PROTOCOL_ID;

  // Byte 2 and 3: Device ID
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;

  // Byte 4: Zone
  packet[packetPtr++] = groupId;

  // Byte 5: Bulb command, filled in later
  packet[packetPtr++] = 0;

  // Byte 6: Packet sequence number 0..255
  packet[packetPtr++] = sequenceNum++;

  // Byte 7: Checksum over previous bytes, including packet length = 7
  // The checksum will be calculated when setting the command field
  packet[packetPtr++] = 0;

  // Byte 8: CRC LSB
  // Byte 9: CRC MSB
}

void FUT006PacketFormatter::finalizePacket(uint8_t* packet) {
  uint8_t checksum;

  // Calculate checksum over packet length .. sequenceNum
  checksum = 7; // Packet length is not part of packet
  for (uint8_t i = 0; i < 6; i++) {
    checksum += currentPacket[i];
  }
  // Store the checksum in the sixth byte
  currentPacket[6] = checksum;
}

void FUT006PacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, REMOTE_TYPE_FUT006);
  int8_t knownValue = (state != NULL && state->isSetBrightness()) ? state->getBrightness() / FUT006_INTERVALS : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    FUT006_INTERVALS,
    value / FUT006_INTERVALS,
    knownValue
  );
}

void FUT006PacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[FUT006_COMMAND_INDEX] = command;
}

void FUT006PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(getStatusButton(groupId, status), 0);
}

void FUT006PacketFormatter::increaseBrightness() {
  command(FUT006_BRIGHTNESS_UP, 0);
}

void FUT006PacketFormatter::decreaseBrightness() {
  command(FUT006_BRIGHTNESS_DOWN, 0);
}

void FUT006PacketFormatter::enableNightMode() {
  command(getStatusButton(groupId, OFF) | 0x10, 0);
}

uint8_t FUT006PacketFormatter::getStatusButton(uint8_t groupId, MiLightStatus status) {
  uint8_t button = 0;

  if (status == ON) {
    switch(groupId) {
      case 0:
        button = FUT006_ALL_ON;
        break;
      case 1:
        button = FUT006_GROUP_1_ON;
        break;
      case 2:
        button = FUT006_GROUP_2_ON;
        break;
      case 3:
        button = FUT006_GROUP_3_ON;
        break;
      case 4:
        button = FUT006_GROUP_4_ON;
        break;
    }
  } else {
    switch(groupId) {
      case 0:
        button = FUT006_ALL_OFF;
        break;
      case 1:
        button = FUT006_GROUP_1_OFF;
        break;
      case 2:
        button = FUT006_GROUP_2_OFF;
        break;
      case 3:
        button = FUT006_GROUP_3_OFF;
        break;
      case 4:
        button = FUT006_GROUP_4_OFF;
        break;
    }
  }

  return button;
}

uint8_t FUT006PacketFormatter::commandIdToGroup(uint8_t command) {
  switch (command & 0xF) {
    case FUT006_GROUP_1_ON:
    case FUT006_GROUP_1_OFF:
      return 1;
    case FUT006_GROUP_2_ON:
    case FUT006_GROUP_2_OFF:
      return 2;
    case FUT006_GROUP_3_ON:
    case FUT006_GROUP_3_OFF:
      return 3;
    case FUT006_GROUP_4_ON:
    case FUT006_GROUP_4_OFF:
      return 4;
    case FUT006_ALL_ON:
    case FUT006_ALL_OFF:
      return 0;
  }

  return 255;
}

MiLightStatus FUT006PacketFormatter::commandToStatus(uint8_t command) {
  switch (command & 0xF) {
    case FUT006_GROUP_1_ON:
    case FUT006_GROUP_2_ON:
    case FUT006_GROUP_3_ON:
    case FUT006_GROUP_4_ON:
    case FUT006_ALL_ON:
      return ON;
    case FUT006_GROUP_1_OFF:
    case FUT006_GROUP_2_OFF:
    case FUT006_GROUP_3_OFF:
    case FUT006_GROUP_4_OFF:
    case FUT006_ALL_OFF:
    default:
      return OFF;
  }
}

BulbId FUT006PacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  uint8_t command = packet[FUT006_COMMAND_INDEX] & 0x7F;

  uint8_t onOffGroupId = commandIdToGroup(command);
  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    onOffGroupId < 255 ? onOffGroupId : packet[3],
    REMOTE_TYPE_FUT006
  );

  // Night mode
  if (command & 0x10) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
  } else if (onOffGroupId < 255) {
    result[GroupStateFieldNames::STATE] = commandToStatus(command) == ON ? "ON" : "OFF";
  } else if (command == FUT006_BRIGHTNESS_DOWN) {
    result[GroupStateFieldNames::COMMAND] = "brightness_down";
  } else if (command == FUT006_BRIGHTNESS_UP) {
    result[GroupStateFieldNames::COMMAND] = "brightness_up";
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void FUT006PacketFormatter::format(uint8_t const* packet, char* buffer) {
  PacketFormatter::formatV1Packet(packet, buffer);
}
