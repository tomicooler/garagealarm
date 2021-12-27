#include "../common/common.h"
#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

namespace {

static constexpr uint GPIO_INFRA = 22; // GP22 - PIN 29

class DoorWatcher {
public:
  bool init_lora();
  void init_encryption();
  void send_message(std::string message);

private:
  AES_ctx aes_ctx;
  GarageAlarm::Packet packet;
};

} // namespace

int main() {
  stdio_init_all();

  DoorWatcher watcher;
  if (!watcher.init_lora()) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  watcher.init_encryption();

  // TODO deep sleep, wake up on open, etc..
  gpio_init(GPIO_INFRA);

  static int todo = 0;
  while (true) {
    sleep_ms(1000);

    if (gpio_get(GPIO_INFRA) == 0) {
      printf("DOOR CLOSED\n");
      todo = 0;
      watcher.send_message(GarageAlarm::DOOR_CLOSE);
      sleep_ms(3000);
      printf("slept\n");
    } else {
      printf("DOOR OPEN\n");
      if (todo < 2) {
        watcher.send_message(GarageAlarm::DOOR_OPEN);
      }
      sleep_ms(3000);
      printf("slept\n");
      ++todo;
    }
  }
  return 0;
}

bool DoorWatcher::init_lora() { return LoRa.begin(GarageAlarm::FREQUENCY); }

void DoorWatcher::init_encryption() {
  // NOTE: the key should be replaced!
  AES_init_ctx(&aes_ctx, GarageAlarm::PRESHARED_KEY);

  // IV 16 byte [ xx xx xx xx xx xx xx xx cc cc cc cc cc cc cc cc ]
  //  xx xx xx xx xx xx xx xx : random 8 byte using LoRa Wideband RSSI
  //  cc cc cc cc cc cc cc cc : 8 byte is a 64 bit counter
  LoRa.receive();
  for (int i = 0; i < 8; ++i) {
    packet.iv[i] = LoRa.random();
  }
  LoRa.sleep();
}

void DoorWatcher::send_message(std::string message) {
  printf("Sending message '%s'\n", message.c_str());

  AES_ctx_set_iv(&aes_ctx, packet.iv);
  AES_CTR_xcrypt_buffer(&aes_ctx, reinterpret_cast<uint8_t *>(message.data()),
                        message.size());

  packet.payload = std::move(message);
  GarageAlarm::lora_write(LoRa, packet);
  GarageAlarm::inc_iv_bottom(packet);

  LoRa.sleep();
}
