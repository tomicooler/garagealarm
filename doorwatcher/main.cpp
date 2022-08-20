#include "../common/common.h"
#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include <hardware/regs/io_bank0.h>
#include <hardware/xosc.h>
#include <pico/sleep.h>
#include <pico/stdlib.h>
#include <boards/pico.h>
#include <stdio.h>
#include <string.h>

namespace {

static constexpr uint GPIO_MAGNETIC = 22; // GP22 - PIN 29
static constexpr uint LED_PIN = PICO_DEFAULT_LED_PIN;

class DoorWatcher {
public:
  bool init_lora();
  void init_encryption();
  void check_door();

private:
  void dormant_sleep_until();
  void send_message(std::string message);

private:
  AES_ctx aes_ctx;
  GarageAlarm::Packet packet;
};

static void my_sleep_goto_dormant_until_pin(uint gpio, uint32_t event) {
  // Configure the appropriate IRQ at IO bank 0
  assert(gpio < NUM_BANK0_GPIOS);

  gpio_set_dormant_irq_enabled(gpio, event, true);

  xosc_dormant();
  // Execution stops here until woken up

  // Clear the irq so we can go back to dormant mode again if we want
  gpio_acknowledge_irq(GPIO_MAGNETIC, event);
}

} // namespace

int main() {
  stdio_init_all();

  DoorWatcher watcher;
  if (!watcher.init_lora()) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  LoRa.setSpreadingFactor(GarageAlarm::SPREADING_FACTOR);
  LoRa.setSignalBandwidth(GarageAlarm::BANDWIDTH);
  LoRa.setCodingRate4(8);
  LoRa.setTxPower(20);
  LoRa.enableCrc();

  watcher.init_encryption();

  gpio_init(GPIO_MAGNETIC);
  gpio_set_pulls(GPIO_MAGNETIC, true, false);
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  while (true) {
    printf(">> Checking\n");
    watcher.check_door();
    printf("<< Woke up\n");
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
  printf("    Sending message '%s'\n", message.c_str());

  AES_ctx_set_iv(&aes_ctx, packet.iv);
  AES_CTR_xcrypt_buffer(&aes_ctx, reinterpret_cast<uint8_t *>(message.data()),
                        message.size());

  packet.payload = std::move(message);
  GarageAlarm::lora_write(LoRa, packet);
  GarageAlarm::inc_iv_bottom(packet);

  LoRa.sleep();
}

void DoorWatcher::check_door() {
  gpio_put(LED_PIN, 1);
  if (gpio_get(GPIO_MAGNETIC) == 0) {
    printf("  DOOR CLOSE\n");
    send_message(GarageAlarm::DOOR_CLOSE);
  } else {
    printf("  DOOR OPEN\n");
    send_message(GarageAlarm::DOOR_OPEN);
  }
  gpio_put(LED_PIN, 0);
  dormant_sleep_until();
}

void DoorWatcher::dormant_sleep_until() {
  printf("Switching to XOSC\n");
  uart_default_tx_wait_blocking();

  // UART will be reconfigured by sleep_run_from_xosc
  sleep_run_from_xosc();

  printf("Running from XOSC\n");
  uart_default_tx_wait_blocking();

  printf("XOSC going dormant\n");
  stdio_flush();
  uart_default_tx_wait_blocking();

  my_sleep_goto_dormant_until_pin(
      GPIO_MAGNETIC, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_HIGH_BITS |
                      IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS);
}
