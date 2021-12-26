#include "../third_party/pico-lora/src/LoRa-RP2040.h"
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

void onReceive(int packetSize) {
  printf("Received packet size %d \n", packetSize);

  std::string str;
  str.reserve(packetSize);
  while (LoRa.available()) {
    str += (char)LoRa.read();
  }
  hexdump(str.c_str(), str.size());

  printf("RSSI %d\n", LoRa.packetRssi());
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

  while (true) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      onReceive(packetSize);
    }
  }

  printf("exit\n");

  return 0;
}
