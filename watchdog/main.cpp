#include "../common/common.h"
#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include <functional>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

#include "action.h"

#ifdef PICO_WIFI
#include "dateutils.h"
#include "dns.h"
#include "fcm.h"
#include "ntp.h"
#include "oauth2.h"
#include "poller.h"
#include "tcpclient.h"
#include "watchdog-config.h" // check the watchdog-config.h.example
#include <hardware/rtc.h>
#include <inttypes.h>
#include <pico/cyw43_arch.h>
#endif

namespace {

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

  for (int i = 0; i < 10; ++i) {
    Poller::led_on();
    sleep_ms(250);
    Poller::led_off();
    sleep_ms(250);
  }

  Ntp ntp;
  Dns dns;
  OAuth2 oauth2{OAuth2::Config{
      CLIENT_EMAIL, PRIVATE_KEY_ID,
      std::string{PRIVATE_KEY, sizeof(PRIVATE_KEY) / sizeof(const char)},
      PROJECT_ID, TOKEN_URI, SCOPE}};
  FCM fcm{PROJECT_ID, DEVICE_TOKEN};

  TcpClient client;

  for (int i = 0; i < 5; ++i) {
    printf("Initialising time from NTP %d\n", i);
    ntp.loop();
  }

  uint64_t last_oauth_timestamp = 0;
  uint64_t last_expires_in = 3599;
  std::string auth_token;

  WatchDog watchdog([&](Action action) {
    Poller::led_on();
    print_action(action);

    // OAuth2 refresh token
    const auto now = DateUtils::unixnowseconds();
    printf("OAuth2 now='%" PRIu64 "' last_oauth_timestamp='%" PRIu64
           "' last_expires_in='%" PRIu64 "' token='%s'\n",
           now, last_oauth_timestamp, last_expires_in, auth_token.c_str());
    if (now - last_oauth_timestamp > last_expires_in) {
      auth_token.clear();
      if (const auto address = dns.lookup("oauth2.googleapis.com"); address) {
        client.send_message(*address, 443,
                            HTTP::write(oauth2.create_request()));
        while (!client.is_finished()) {
          printf(" ... waiting to finish oauth2 ... polling\n");
          Poller::poll();
        }

        const auto raw_http_response = client.get_last_response();
        printf("=== %d\n", raw_http_response.size());
        printf("%s\n", raw_http_response.c_str());
        printf("======\n");
        if (const auto oauth_response =
                OAuth2::parse_response(HTTP::read(raw_http_response));
            oauth_response) {
          last_oauth_timestamp = DateUtils::unixnowseconds();
          last_expires_in = std::max(0, oauth_response->expires_in);
          auth_token = oauth_response->access_token;
        }
      } else {
        printf("Could not resolve oauth2.googleapis.com\n");
      }
    }

    // FCM send message
    if (!auth_token.empty()) {
      if (const auto address = dns.lookup("fcm.googleapis.com"); address) {
        client.send_message(
            *address, 443, HTTP::write(fcm.create_request(action, auth_token)));
        while (!client.is_finished()) {
          printf(" ... waiting to finish fcm ... polling\n");
          Poller::poll();
        }
      } else {
        printf("Could not resolve fmc.googleapis.com\n");
      }
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
  LoRa.setCodingRate4(GarageAlarm::CODING_RATE);
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
