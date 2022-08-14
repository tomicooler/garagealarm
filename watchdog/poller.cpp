#include "poller.h"
#include "dateutils.h"

#include <hardware/rtc.h>
#include <pico/cyw43_arch.h>

void Poller::poll() {
#if PICO_CYW43_ARCH_POLL
  cyw43_arch_poll();
  sleep_ms(1);
#else
  sleep_ms(500);
#endif

  datetime_t t;
  rtc_get_datetime(&t);

  printf("::: poll datetime %llu\n", DateUtils::to_epoch(t));
}

void Poller::led_on() { cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); }

void Poller::led_off() { cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0); }
