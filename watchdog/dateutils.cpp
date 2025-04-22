#include "dateutils.h"

#include <hardware/rtc.h>
#include <pico/util/datetime.h>

// Credits to
// https://github.com/micropython/micropython/blob/ca41eda281052ecf64374f35dcf282a96e68661a/shared/timeutils/timeutils.h

namespace {
static constexpr uint64_t TIMEUTILS_SECONDS_1970_TO_2000 = 946684800ULL;

static constexpr uint16_t days_since_jan1[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};

bool timeutils_is_leap_year(uint16_t year) {
  return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

uint16_t timeutils_year_day(uint16_t year, uint16_t month, uint16_t date) {
  uint16_t yday = days_since_jan1[month - 1] + date;
  if (month >= 3 && timeutils_is_leap_year(year)) {
    yday += 1;
  }
  return yday;
}

uint64_t timeutils_seconds_since_2000(uint64_t year, uint64_t month,
                                      uint64_t date, uint64_t hour,
                                      uint64_t minute, uint64_t second) {
  return second + minute * 60 + hour * 3600 +
         (timeutils_year_day(year, month, date) - 1 +
          ((year - 2000 + 3) / 4) // add a day each 4 years starting with 2001
          - ((year - 2000 + 99) /
             100) // subtract a day each 100 years starting with 2001
          + ((year - 2000 + 399) /
             400) // add a day each 400 years starting with 2001
          ) * 86400 +
         (year - 2000) * 31536000;
}
} // namespace

uint64_t DateUtils::to_epoch(const datetime_t &t) {
  return timeutils_seconds_since_2000(t.year, t.month, t.day, t.hour, t.min,
                                      t.sec) +
         TIMEUTILS_SECONDS_1970_TO_2000;
}

uint64_t DateUtils::unixnowseconds() {
  datetime_t t;
  rtc_get_datetime(&t);
  return DateUtils::to_epoch(t);
}
