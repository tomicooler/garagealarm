#ifndef GARAGE_ALARM_COMMON
#define GARAGE_ALARM_COMMON

#include <cstdint>

namespace GarageAlarm {

static constexpr uint8_t PRESHARED_KEY[] = "replacethis!";
static constexpr uint8_t MESSAGE_HEADER{0x42};

} // namespace GarageAlarm

#endif
