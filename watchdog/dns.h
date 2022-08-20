#ifndef GARAGE_WATCHDOG_DNS
#define GARAGE_WATCHDOG_DNS

#include <lwip/ip_addr.h>
#include <optional>
#include <string>

struct Dns {
  std::optional<ip_addr_t> lookup(const std::string &address,
                                  int max_retry = 5);

private:
  ip_addr_t addr{0};
  bool success{false};
};

#endif // GARAGE_WATCHDOG_DNS
