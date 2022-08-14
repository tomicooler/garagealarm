#ifndef GARAGE_WATCHDOG_DATEUTILS
#define GARAGE_WATCHDOG_DATEUTILS

#include <pico/types.h>

namespace DateUtils {

uint64_t to_epoch(const datetime_t &t);

} // namespace DateUtils

#endif // GARAGE_WATCHDOG_DATEUTILS
