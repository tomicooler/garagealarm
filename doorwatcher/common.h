#ifndef GARAGE_ALARM_COMMON
#define GARAGE_ALARM_COMMON

#include <cstdint>

namespace GarageAlarm {

static constexpr uint8_t PRESHARED_KEY[] = "replacethis";
static constexpr uint8_t MESSAGE_HEADER{0x42};

#ifdef NDEBUG
constexpr bool ensure_replaced() {
  constexpr uint8_t DONT_USE_THIS[] = "replacethis";
  for (int i = 0; i < sizeof(DONT_USE_THIS); ++i) {
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
