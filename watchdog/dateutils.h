#ifndef GARAGE_WATCHDOG_DATEUTILS
#define GARAGE_WATCHDOG_DATEUTILS

#include <pico/types.h>

namespace DateUtils {

uint64_t to_epoch(const datetime_t &t);
uint64_t unixnowseconds();

} // namespace DateUtils

#endif // GARAGE_WATCHDOG_DATEUTILS
