#ifndef GARAGE_WATCHDOG_TCPCLIENT
#define GARAGE_WATCHDOG_TCPCLIENT

#include <lwip/ip_addr.h>
#include <memory>
#include <string>

struct TcpClientPrivate;

struct TcpClient {
  TcpClient();
  ~TcpClient();

  bool is_finished() const;
  bool send_message(const ip_addr_t &ip, const int port, std::string &&message);

private:
  std::unique_ptr<TcpClientPrivate> d;
};

#endif // GARAGE_WATCHDOG_TCPCLIENT
