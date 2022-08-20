#include "ntp.h"

#include "dateutils.h"
#include "dns.h"

#include <hardware/rtc.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <optional>
#include <pico/cyw43_arch.h>
#include <pico/time.h>
#include <pico/types.h>
#include <time.h>

namespace {

static constexpr int NTP_MSG_LEN = 48;
static constexpr int NTP_RESEND_TIME = 30 * 1000;
static constexpr int NTP_POLL_TIME = 64 * 1000;

} // namespace

struct NtpPrivate {
  NtpPrivate(std::string &&server, int port);
  ~NtpPrivate();

  void loop();

private:
  void handle_result(int status, time_t *result);
  void ntp_request();

private:
  const std::string server;
  const int port;
  Dns dns;
  ip_addr_t server_address{0};

  udp_pcb *pcb{udp_new_ip_type(IPADDR_TYPE_ANY)};
  std::optional<absolute_time_t> test_time;
  alarm_id_t resend_alarm;
};

Ntp::Ntp(std::string &&server, int port)
    : d(std::make_unique<NtpPrivate>(std::move(server), port)) {}

Ntp::~Ntp() {}

void Ntp::loop() { d->loop(); }

NtpPrivate::NtpPrivate(std::string &&server, int port)
    : server(std::move(server)), port(port) {
  if (!pcb) {
    printf("ntp# failed to create pcb\n");
    return;
  }
  udp_recv(
      pcb,
      +[](void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr,
          u16_t port) {
        auto ntp = reinterpret_cast<NtpPrivate *>(arg);
        uint8_t mode = pbuf_get_at(p, 0) & 0x7;
        uint8_t stratum = pbuf_get_at(p, 1);

        if (ip_addr_cmp(addr, &ntp->server_address) && port == ntp->port &&
            p->tot_len == NTP_MSG_LEN && mode == 0x4 && stratum != 0) {
          uint8_t seconds_buf[4] = {0};
          pbuf_copy_partial(p, seconds_buf, sizeof(seconds_buf), 40);
          static const uint64_t seconds_between_1_jan_1900_to_1970 = 2208988800;
          uint32_t seconds_since_1900 = seconds_buf[0] << 24 |
                                        seconds_buf[1] << 16 |
                                        seconds_buf[2] << 8 | seconds_buf[3];
          uint32_t seconds_since_1970 =
              seconds_since_1900 - seconds_between_1_jan_1900_to_1970;
          time_t epoch = seconds_since_1970;
          ntp->handle_result(0, &epoch);
        } else {
          printf("ntp# invalid ntp response\n");
          ntp->handle_result(-1, NULL);
        }
        pbuf_free(p);
      },
      this);
}

NtpPrivate::~NtpPrivate() {
  if (pcb) {
    udp_remove(pcb);
  }
}

void NtpPrivate::loop() {
  if (!test_time ||
      absolute_time_diff_us(get_absolute_time(), *test_time) < 0) {
    printf("ntp# loop\n");
    if (!test_time) {
      test_time = make_timeout_time_ms(NTP_RESEND_TIME);
    }

    if (const auto address = dns.lookup(server); address) {
      this->server_address = *address;
      ntp_request();
    } else {
      printf("ntp# could not resolve %s\n", server.c_str());
    }

    resend_alarm = add_alarm_in_ms(
        NTP_RESEND_TIME,
        +[](alarm_id_t id, void *arg) -> int64_t {
          printf("ntp# request failed\n");
          auto ntp = reinterpret_cast<NtpPrivate *>(arg);
          ntp->handle_result(-1, NULL);
          return 0;
        },
        this, true);
  }
}

void NtpPrivate::handle_result(int status, time_t *result) {
  if (status == 0 && result) {
    struct tm *utc = gmtime(result);
    printf("ntp# response: %02d/%02d/%04d %02d:%02d:%02d\n", utc->tm_mday,
           utc->tm_mon + 1, utc->tm_year + 1900, utc->tm_hour, utc->tm_min,
           utc->tm_sec);

    datetime_t t = {
        .year = static_cast<int16_t>(utc->tm_year + 1900),
        .month = static_cast<int8_t>(utc->tm_mon + 1),
        .day = static_cast<int8_t>(utc->tm_mday),
        .dotw = static_cast<int8_t>(utc->tm_wday),
        .hour = static_cast<int8_t>(utc->tm_hour),
        .min = static_cast<int8_t>(utc->tm_min),
        .sec = static_cast<int8_t>(utc->tm_sec),
    };

    printf("ntp# as datetime %llu\n", DateUtils::to_epoch(t));
    rtc_set_datetime(&t);
  }

  if (resend_alarm > 0) {
    cancel_alarm(resend_alarm);
    resend_alarm = 0;
  }

  test_time = make_timeout_time_ms(NTP_POLL_TIME);
}

void NtpPrivate::ntp_request() {
  cyw43_arch_lwip_begin();
  pbuf *p = pbuf_alloc(PBUF_TRANSPORT, NTP_MSG_LEN, PBUF_RAM);
  uint8_t *req = (uint8_t *)p->payload;
  memset(req, 0, NTP_MSG_LEN);
  req[0] = 0x1b;
  udp_sendto(pcb, p, &server_address, port);
  pbuf_free(p);
  cyw43_arch_lwip_end();
}
