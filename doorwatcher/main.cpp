#include "pico/stdlib.h"
#include <stdio.h>

static constexpr uint GPIO_INFRA = 22; // GP22 - PIN 29

int main() {
  stdio_init_all();
  gpio_init(GPIO_INFRA);

  int c = 0;
  while (true) {
    printf("Hello, world! %d  infra %d\n", c, gpio_get(GPIO_INFRA));
    sleep_ms(1000);
    ++c;
  }
  return 0;
}
