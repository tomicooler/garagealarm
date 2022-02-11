#ifndef GARAGE_ALARM_COMMON
#define GARAGE_ALARM_COMMON

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class LoRaClass;

namespace Utils {

static void hexdump(const void *const ptr, int buflen) {
  const unsigned char *buf = reinterpret_cast<const unsigned char *>(ptr);
  int i, j;
  for (i = 0; i < buflen; i += 16) {
    printf("%06x: ", i);
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%02x ", buf[i + j]);
      else
        printf("   ");
    printf(" ");
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
    printf("\n");
  }
}

} // namespace Utils

namespace GarageAlarm {

static constexpr long FREQUENCY = 433E6;

static constexpr uint8_t PRESHARED_KEY[] = "replacethis";

static constexpr uint8_t MESSAGE_HEADER = 0x42;

static constexpr char DOOR_OPEN[] = "open_";
static constexpr char DOOR_CLOSE[] = "close";

struct Packet {
  uint8_t header{GarageAlarm::MESSAGE_HEADER};
  uint8_t iv[16]{};
  std::string payload{};
};

uint64_t iv_top(const Packet &packet);
uint64_t iv_bottom(const Packet &packet);
void inc_iv_bottom(Packet &packet);

void lora_write(LoRaClass &lora, const Packet &packet);
std::optional<std::vector<uint8_t>> lora_read(LoRaClass &lora);
std::optional<Packet> parse_packet(const std::vector<uint8_t> &bytes);

#ifdef NDEBUG
constexpr bool ensure_replaced() {
  constexpr uint8_t DONT_USE_THIS[] = "replacethis";
  for (unsigned int i = 0; i < sizeof(DONT_USE_THIS); ++i) {
    if (PRESHARED_KEY[i] != DONT_USE_THIS[i]) {
      return true;
    }
  }
  return false;
}

static_assert(sizeof(PRESHARED_KEY) == 12,
              "The AES preshared key size must be 12!");

static_assert(ensure_replaced(),
              "Replace the AES preshared key with a random 12 byte key!");
#endif

} // namespace GarageAlarm

#endif
