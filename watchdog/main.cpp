#include "../third_party/pico-lora/src/LoRa-RP2040.h"
#include "../third_party/tiny-AES-c/aes.hpp"
#include "common.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

void hexdump(const char *ptr, int buflen) {
  unsigned char *buf = (unsigned char *)ptr;
  int i, j;
  for (i = 0; i < buflen; i += 16) {
    printf("%06x: ", i);
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%02x ", buf[i + j]);
      else
        printf("   ");
    printf(" ");
    for (j = 0; j < 16; j++)
      if (i + j < buflen)
        printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
    printf("\n");
  }
}

int main() {
  stdio_init_all();

  for (int i = 0; i < 3; ++i) {
    printf("i: %d\n", i);
    sleep_ms(1000);
  }

  if (!LoRa.begin(868E6)) {
    printf("Starting LoRa failed!\n");
    return 1;
  }

  printf("LoRa Started\n");

  // TODO: investigate it only receives 1 packet
  //       might be related to this?
  //       https://forums.raspberrypi.com/viewtopic.php?t=300430
  // LoRa.onReceive(onReceive);
  LoRa.receive();

  AES_ctx aes_ctx;
  AES_init_ctx(&aes_ctx, GarageAlarm::PRESHARED_KEY);
  static const int HEADER_SIZE = sizeof(GarageAlarm::MESSAGE_HEADER);
  static const int FRAMING_SIZE = (HEADER_SIZE + AES_BLOCKLEN);
  while (true) {
    int packet_size = LoRa.parsePacket();
    if (packet_size) {
      printf("Received packet size %d %d \n", packet_size, FRAMING_SIZE);

      std::string packet;
      packet.reserve(packet_size);
      while (LoRa.available()) {
        packet += (char)LoRa.read();
      }
      hexdump(packet.c_str(), packet.size());

      printf("RSSI %d\n", LoRa.packetRssi());

      if (packet_size > FRAMING_SIZE) {
        if (static_cast<uint8_t>(packet[0]) == GarageAlarm::MESSAGE_HEADER) {
          AES_ctx_set_iv(&aes_ctx, reinterpret_cast<const uint8_t *>(
                                       packet.c_str() + HEADER_SIZE));
          AES_CTR_xcrypt_buffer(
              &aes_ctx,
              reinterpret_cast<uint8_t *>(packet.data() + FRAMING_SIZE),
              packet.size() - FRAMING_SIZE);

          printf("Decrypted: \n");
          hexdump(packet.c_str() + FRAMING_SIZE, packet.size() - FRAMING_SIZE);
        }
      }
    }
  }

  printf("exit\n");

  return 0;
}
