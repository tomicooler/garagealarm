#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

static constexpr uint GPIO_INFRA = 22; // GP22 - PIN 29

int main() {
  stdio_init_all();

  if (!LoRa.begin(868E6)) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  gpio_init(GPIO_INFRA);

  uint8_t counter = 0;
  while (true) {
    printf("Hello, world! %d  infra %d\n", counter, gpio_get(GPIO_INFRA));
    sleep_ms(1000);
    ++counter;

    if (gpio_get(GPIO_INFRA) == 0) {
      printf("Sending packet: ");
      printf("%d \n", counter);

      LoRa.beginPacket();
      std::string hello{"hello"};
      LoRa.print(hello);
      LoRa.endPacket();
      printf("packet sent\n");

      sleep_ms(3000);
      printf("slept\n");
    }
  }
  return 0;
}
