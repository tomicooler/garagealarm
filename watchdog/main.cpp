#include "../common/common.h"
#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include <functional>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef PICO_WIFI
#include "dateutils.h"
#include "dns.h"
#include "ntp.h"
#include "poller.h"
#include "tcpclient.h"
#include "watchdog-config.h" // check the watchdog-config.h.example
#include <hardware/rtc.h>
#include <pico/cyw43_arch.h>
#endif

namespace {

enum class Action { DoorOpen, DoorClose, IvTopMismatch, IvBottomOld };

#ifdef PICO_WIFI
std::string action_to_string(Action action) {
  switch (action) {
  case Action::DoorOpen:
    return std::string{"open"};
  case Action::DoorClose:
    return std::string{"close"};
  case Action::IvTopMismatch:
    return std::string{"iv_top_mismatch"};
  case Action::IvBottomOld:
    return std::string{"iv_bottom_old"};
  }
  return std::string{};
}

std::string create_http_request(Action action) {
  datetime_t t;
  rtc_get_datetime(&t);

  std::string content = R"({"to": ")" + std::string{FIREBASE_DEVICE_TOKEN} +
                        R"(", "priority": "high", "data": {"timestamp": )" +
                        std::to_string(DateUtils::to_epoch(t) * 1000) +
                        R"(, "action": ")" + action_to_string(action) +
                        R"("}})";

  std::string request = "POST /fcm/send HTTP/1.1\r\n"
                        "Host: fcm.googleapis.com\r\n"
                        "Connection: close\r\n"
                        "Content-type: application/json\r\n"
                        "Authorization: key=" +
                        std::string{FIREBASE_SERVER_KEY} +
                        "\r\n"
                        "Content-Length: " +
                        std::to_string(content.size()) +
                        "\r\n"
                        "\r\n" +
                        content + "\r\n";
  return request;
}
#endif

std::optional<Action> get_action(const std::string &message) {
  printf(">> decoded message >>\n");
  Utils::hexdump(message.c_str(), message.size());
  printf("<< decoded message <<\n");
  if (message == GarageAlarm::DOOR_OPEN) {
    return Action::DoorOpen;
  } else if (message == GarageAlarm::DOOR_CLOSE) {
    return Action::DoorClose;
  }
  return std::nullopt;
}

void print_action(Action action) {
  switch (action) {
  case Action::DoorOpen:
    printf("#ACTION_DOOR_OPEN#\n");
    break;
  case Action::DoorClose:
    printf("#ACTION_DOOR_CLOSE#\n");
    break;
  case Action::IvTopMismatch:
    printf("#IV_TOP_MISMATCH#\n");
    break;
  case Action::IvBottomOld:
    printf("#IV_BOTTOM_OLD#\n");
    break;
  }
}

struct IvTopBottom {
  uint64_t top{};
  uint64_t bottom{};
};

class WatchDog {
public:
  explicit WatchDog(std::function<void(Action)> action_handler);

  void watch();

private:
  void handle_packet(const GarageAlarm::Packet &packet);

private:
  AES_ctx aes_ctx;
  std::optional<IvTopBottom> iv{};
  std::function<void(Action)> action_handler;
};

} // namespace

int main() {
  stdio_init_all();

#ifdef PICO_WIFI
  datetime_t t = {
      .year = 2022,
      .month = 8,
      .day = 14,
      .dotw = 7,
      .hour = 0,
      .min = 0,
      .sec = 0,
  };
  rtc_init();
  rtc_set_datetime(&t);

  sleep_ms(2000);
  printf("Starting...\n");

  if (const auto err = cyw43_arch_init_with_country(CYW43_COUNTRY_HUNGARY);
      err) {
    printf("Failed to initialise cyw43 %d\n", err);
    return 1;
  }
  printf("Initialized cyw43\n");

  cyw43_arch_enable_sta_mode();

  if (int err = cyw43_arch_wifi_connect_timeout_ms(
          WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000);
      err != ERR_OK) {
    cyw43_arch_deinit();
    printf("Failed to connect to WiFi %d\n", err);
    return 1;
  }
  printf("Connected to WiFi\n");

  Ntp ntp;
  Dns dns;
  TcpClient client;

  for (int i = 0; i < 5; ++i) {
    printf("Initialising time from NTP %d\n", i);
    ntp.loop();
  }

  WatchDog watchdog([&](Action action) {
    Poller::led_on();
    print_action(action);
    if (const auto address = dns.lookup("fcm.googleapis.com"); address) {
      client.send_message(*address, 443, create_http_request(action));
    } else {
      printf("Could not resolve fmc.googleapis.com\n");
    }

    while (!client.is_finished()) {
      printf(" ... waiting to finish sending ... polling\n");
      Poller::poll();
    }
    Poller::led_off();
  });
#else
  WatchDog watchdog(print_action);
#endif

  printf("Watchdog created\n");

  if (!LoRa.begin(GarageAlarm::FREQUENCY)) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  LoRa.setSpreadingFactor(GarageAlarm::SPREADING_FACTOR);
  LoRa.setSignalBandwidth(GarageAlarm::BANDWIDTH);
  LoRa.enableCrc();

  LoRa.receive();

  printf("LoRa initialised\n");

  while (true) {
    watchdog.watch();
#ifdef PICO_WIFI
    ntp.loop();
#endif
  }

#ifdef PICO_WIFI
  cyw43_arch_deinit();
#endif

  return 0;
}

WatchDog::WatchDog(std::function<void(Action)> action_handler)
    : action_handler(action_handler) {
  AES_init_ctx(&aes_ctx, GarageAlarm::PRESHARED_KEY);
}

void WatchDog::watch() {
  if (const auto data = GarageAlarm::lora_read(LoRa); data) {
    if (auto packet = GarageAlarm::parse_packet(*data); packet) {
      handle_packet(*packet);
    }
  }
}

void WatchDog::handle_packet(const GarageAlarm::Packet &packet) {
  if (packet.header == GarageAlarm::MESSAGE_HEADER) {
    AES_ctx_set_iv(&aes_ctx, packet.iv);
    std::string message = std::move(packet.payload);
    AES_CTR_xcrypt_buffer(&aes_ctx, reinterpret_cast<uint8_t *>(message.data()),
                          message.size());

    auto action = get_action(message);
    if (action) {
      IvTopBottom iv_received{GarageAlarm::iv_top(packet),
                              GarageAlarm::iv_bottom(packet)};
      if (iv) {
        // Non-repetition as-is
        // Make sure the manually that the very first message
        // is read from a trusted source
        if (iv->top != iv_received.top) {
          action = Action::IvTopMismatch;
        } else if (iv->bottom < iv_received.bottom) {
          iv->bottom = iv_received.bottom;
        } else {
          action = Action::IvBottomOld;
        }
        action_handler(*action);
      } else {
        iv = std::move(iv_received);
        action_handler(*action);
      }
    }
    stdio_flush();
  }
}
