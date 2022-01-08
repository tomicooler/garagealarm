#include "../common/common.h"
#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

namespace {

enum class Action { Invalid, DoorOpen, DoorClose };

Action get_action(const std::string &message) {
  printf(">> decoded message >>\n");
  Utils::hexdump(message.c_str(), message.size());
  printf("<< decoded message <<\n");
  if (message == GarageAlarm::DOOR_OPEN) {
    return Action::DoorOpen;
  } else if (message == GarageAlarm::DOOR_CLOSE) {
    return Action::DoorClose;
  }
  return Action::Invalid;
}

void print_action(Action action) {
  switch (action) {
  case Action::DoorOpen:
    printf("#ACTION_DOOR_OPEN#\n");
    break;
  case Action::DoorClose:
    printf("#ACTION_DOOR_CLOSE#\n");
    break;
  default:
    break;
  }
}

struct IvTopBottom {
  uint64_t top{};
  uint64_t bottom{};
};

class WatchDog {
public:
  explicit WatchDog();

  bool init_lora();
  void watch();

private:
  void handle_packet(const GarageAlarm::Packet &packet);

private:
  AES_ctx aes_ctx;
  std::optional<IvTopBottom> iv{};
};

} // namespace

int main() {
  stdio_init_all();

  WatchDog watchdog;
  if (!watchdog.init_lora()) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  LoRa.setGain(6);
  LoRa.setSpreadingFactor(12);
  LoRa.enableCrc();

  LoRa.receive();
  watchdog.watch();

  return 0;
}

WatchDog::WatchDog() { AES_init_ctx(&aes_ctx, GarageAlarm::PRESHARED_KEY); }

bool WatchDog::init_lora() { return LoRa.begin(GarageAlarm::FREQUENCY); }

void WatchDog::watch() {
  while (true) {
    if (const auto data = GarageAlarm::lora_read(LoRa); data) {
      if (auto packet = GarageAlarm::parse_packet(*data); packet) {
        handle_packet(*packet);
      }
    }
  }
}

void
WatchDog::handle_packet(const GarageAlarm::Packet &packet)
{
  if (packet.header == GarageAlarm::MESSAGE_HEADER) {
    AES_ctx_set_iv(&aes_ctx, packet.iv);
    std::string message = std::move(packet.payload);
    AES_CTR_xcrypt_buffer(&aes_ctx,
                          reinterpret_cast<uint8_t *>(message.data()),
                          message.size());

    Action action = get_action(message);
    if (action != Action::Invalid) {
      IvTopBottom iv_received{GarageAlarm::iv_top(packet),
                              GarageAlarm::iv_bottom(packet)};
      if (iv) {
        // Non-repudiation as-is
        // Make sure the manually that the very first message
        // is read from a trusted source
        if (iv->top != iv_received.top) {
          printf("#IV_TOP_MISMATCH#\n");
        } else if (iv->bottom < iv_received.bottom) {
          iv->bottom = iv_received.bottom;
          print_action(action);
        } else {
          printf("#IV_BOTTOM_OLD#\n");
        }
      } else {
        iv = std::move(iv_received);
        print_action(action);
      }
    }
    stdio_flush();
  }
}
