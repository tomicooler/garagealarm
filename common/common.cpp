#include "common.h"

#include "../third_party/pico-lora/src/LoRa-RP2040.h"

namespace {

uint64_t to(const uint8_t *bytes) {
  return (static_cast<uint64_t>(bytes[7]) << 8 * 0) |
         (static_cast<uint64_t>(bytes[6]) << 8 * 1) |
         (static_cast<uint64_t>(bytes[5]) << 8 * 2) |
         (static_cast<uint64_t>(bytes[4]) << 8 * 3) |
         (static_cast<uint64_t>(bytes[3]) << 8 * 4) |
         (static_cast<uint64_t>(bytes[2]) << 8 * 5) |
         (static_cast<uint64_t>(bytes[1]) << 8 * 6) |
         (static_cast<uint64_t>(bytes[0]) << 8 * 7);
}

} // namespace

void GarageAlarm::lora_write(LoRaClass &lora, const Packet &packet) {
  lora.beginPacket();
  lora.write(packet.header);
  lora.write(packet.iv, sizeof(packet.iv));
  lora.print(packet.payload);
  lora.endPacket();
}

std::optional<std::vector<uint8_t>> GarageAlarm::lora_read(LoRaClass &lora) {
  int packet_size = LoRa.parsePacket();
  if (packet_size) {
    printf("Received packet size: %d\n", packet_size);

    std::vector<uint8_t> data;
    data.reserve(packet_size);
    while (lora.available()) {
      data.push_back(static_cast<uint8_t>(lora.read()));
    }
    Utils::hexdump(data.data(), data.size());

    printf("RSSI %d\n", lora.packetRssi());
    printf("SNR %f\n", lora.packetSnr());
    printf("FreqError %ld\n", lora.packetFrequencyError());

    return data;
  }
  return std::nullopt;
}

std::optional<GarageAlarm::Packet>
GarageAlarm::parse_packet(const std::vector<uint8_t> &bytes) {
  static const int HEADER_SIZE = sizeof(uint8_t);
  static const int IV_SIZE = 16;
  static const int FRAMING_SIZE = (HEADER_SIZE + IV_SIZE);

  if (bytes.size() > FRAMING_SIZE) {
    std::optional<Packet> packet;
    return Packet{
        bytes[0],
        {bytes[1], bytes[2], bytes[3], bytes[4], bytes[5], bytes[6], bytes[7],
         bytes[8], bytes[9], bytes[10], bytes[11], bytes[12], bytes[14],
         bytes[14], bytes[15], bytes[16]},
        std::string{reinterpret_cast<const char *>(bytes.data()) + FRAMING_SIZE,
                    bytes.size() - FRAMING_SIZE}};
  }

  return std::nullopt;
}

uint64_t GarageAlarm::iv_top(const Packet &packet) { return to(packet.iv); }

uint64_t GarageAlarm::iv_bottom(const Packet &packet) {
  return to(packet.iv + 8);
}

void GarageAlarm::inc_iv_bottom(Packet &packet) {
  uint64_t bottom = iv_bottom(packet);
  ++bottom;

  packet.iv[8] = bottom >> 8 * 7;
  packet.iv[9] = bottom >> 8 * 6;
  packet.iv[10] = bottom >> 8 * 5;
  packet.iv[11] = bottom >> 8 * 4;
  packet.iv[12] = bottom >> 8 * 3;
  packet.iv[13] = bottom >> 8 * 2;
  packet.iv[14] = bottom >> 8 * 1;
  packet.iv[15] = bottom >> 8 * 0;
}
