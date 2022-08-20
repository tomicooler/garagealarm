#include "dns.h"
#include "poller.h"

#include <lwip/dns.h>
#include <pico/cyw43_arch.h>

std::optional<ip_addr_t> Dns::lookup(const std::string &address,
                                     int max_retry) {
  printf("dns# look up address: %s\n", address.c_str());

  success = false;
  bool dns_request_sent = false;
  int retry_count = 0;
  while (!success && retry_count < max_retry) {
    if (!dns_request_sent) {
      cyw43_arch_lwip_begin();
      printf("dns# look up address: %s retry_count: %d\n", address.c_str(),
             retry_count);
      const int err = dns_gethostbyname(
          address.c_str(), &addr,
          +[](const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
            auto dns = reinterpret_cast<Dns *>(callback_arg);
            printf("dns# resolved address: %s\n", ipaddr_ntoa(ipaddr));
            dns->success = true;
            dns->addr = *ipaddr;
          },
          this);
      cyw43_arch_lwip_end();
      dns_request_sent = true;

      if (err == ERR_OK) {
        printf("dns# already resolved: %s\n", ipaddr_ntoa(&addr));
        success = true;
      } else if (err != ERR_INPROGRESS) {
        printf("dns# request failed\n");
        dns_request_sent = false;
        ++retry_count;
      }
    }
    Poller::poll();
  }

  printf("dns# returning address: %s %d\n", ipaddr_ntoa(&addr), success);

  return success ? std::optional{addr} : std::nullopt;
}
