#ifndef GARAGE_WATCHDOG_NTP
#define GARAGE_WATCHDOG_NTP

#include <memory>
#include <string>

struct NtpPrivate;

struct Ntp {
  explicit Ntp(std::string &&server = {"pool.ntp.org"}, int port = 123);
  ~Ntp();

  void loop();

private:
  std::unique_ptr<NtpPrivate> d;
};

#endif // GARAGE_WATCHDOG_NTP
