#include "tcpclient.h"

#include "../common/common.h"

#include <lwip/altcp.h>
#include <lwip/altcp_tcp.h>
#include <lwip/altcp_tls.h>
#include <lwip/pbuf.h>
#include <lwip/priv/altcp_priv.h>
#include <pico/cyw43_arch.h>

namespace {
static constexpr int TcpPollTime = 5;
}

struct TcpClientPrivate {
  ~TcpClientPrivate() { close(); }

  bool is_finished() const;
  bool send_message(const ip_addr_t &ip, const int port, std::string &&message);
  std::string get_last_response();

private:
  err_t handle_result(err_t err);
  err_t close();
  void send_message();
  void buffer_response(const unsigned char *data, u16_t len);

private:
  altcp_pcb *tcp_pcb{nullptr};
  altcp_allocator_t tls_allocator = {
      altcp_tls_alloc, altcp_tls_create_config_client(nullptr, 0)};
  std::string message;
  std::string last_response;
};

bool TcpClientPrivate::is_finished() const { return tcp_pcb == nullptr; }

bool TcpClientPrivate::send_message(const ip_addr_t &ip, const int port,
                                    std::string &&message) {
  if (!is_finished()) {
    printf("tcp# !finished\n");
    return false;
  }

  this->last_response = {};
  this->message = std::move(message);
  printf("tcp# connect %s %d\n", ipaddr_ntoa(&ip), port);

  tcp_pcb = altcp_new_ip_type(&tls_allocator, IP_GET_TYPE(&ip));
  if (!tcp_pcb) {
    printf("tcp# failed to create pcb\n");
    return false;
  }

  altcp_arg(tcp_pcb, this);

  altcp_poll(
      tcp_pcb,
      +[](void *arg, struct altcp_pcb *tpcb) -> err_t {
        printf("tcp# polling\n");
        auto tcp = reinterpret_cast<TcpClientPrivate *>(arg);
        return tcp->handle_result(-1);
      },
      TcpPollTime * 2);

  altcp_sent(
      tcp_pcb, +[](void *arg, struct altcp_pcb *tpcb, u16_t len) -> err_t {
        printf("tcp# sent %d\n", len);
        return ERR_OK;
      });

  altcp_recv(
      tcp_pcb,
      +[](void *arg, struct altcp_pcb *tpcb, struct pbuf *p,
          err_t err) -> err_t {
        auto tcp = reinterpret_cast<TcpClientPrivate *>(arg);
        if (!p) {
          return tcp->handle_result(-1);
        }
        cyw43_arch_lwip_check();
        if (p->tot_len > 0) {
          printf("tcp# recv %d err %d\n", p->tot_len, err);
          for (struct pbuf *q = p; q != NULL; q = q->next) {
            tcp->buffer_response(
                reinterpret_cast<const unsigned char *>(q->payload), q->len);
            Utils::hexdump(q->payload, q->len);
          }
          altcp_recved(tpcb, p->tot_len);
        }
        pbuf_free(p);
        return ERR_OK;
      });

  altcp_err(
      tcp_pcb, +[](void *arg, err_t err) {
        if (err != ERR_ABRT) {
          printf("tcp# err %d\n", err);
          auto tcp = reinterpret_cast<TcpClientPrivate *>(arg);
          tcp->handle_result(err);
        }
      });

  cyw43_arch_lwip_begin();
  err_t err = altcp_connect(
      tcp_pcb, &ip, port,
      +[](void *arg, struct altcp_pcb *tpcb, err_t err) -> err_t {
        auto tcp = reinterpret_cast<TcpClientPrivate *>(arg);
        if (err != ERR_OK) {
          printf("tcp# connect failed %d\n", err);
          return tcp->handle_result(err);
        }
        printf("tcp# connected\n");
        tcp->send_message();
        return ERR_OK;
      });
  cyw43_arch_lwip_end();

  return err == ERR_OK;
}

std::string TcpClientPrivate::get_last_response() { return last_response; }

err_t TcpClientPrivate::handle_result(err_t err) {
  if (err == 0) {
    printf("tcp# result success\n");
  } else {
    printf("tcp# result failed %d\n", err);
  }
  return close();
}

err_t TcpClientPrivate::close() {
  err_t err = ERR_OK;
  if (tcp_pcb != nullptr) {
    altcp_arg(tcp_pcb, nullptr);
    altcp_poll(tcp_pcb, nullptr, 0);
    altcp_sent(tcp_pcb, nullptr);
    altcp_recv(tcp_pcb, nullptr);
    altcp_err(tcp_pcb, nullptr);
    err = altcp_close(tcp_pcb);
    if (err != ERR_OK) {
      printf("tcp# close failed %d, calling abort\n", err);
      altcp_abort(tcp_pcb);
      err = ERR_ABRT;
    }
    // altcp_free(tcp_pcb); TODO check leak?
    tcp_pcb = nullptr;
  }
  return err;
}

void TcpClientPrivate::send_message() {
  printf("tcp# sending: %d\n", message.size());
  Utils::hexdump(message.c_str(), message.size());

  cyw43_arch_lwip_check();
  if (const err_t error = altcp_write(tcp_pcb, message.c_str(), message.size(),
                                      TCP_WRITE_FLAG_COPY);
      error) {
    printf("tcp# write error: %d\n", error);
  }

  if (const err_t error = altcp_output(tcp_pcb); error) {
    printf("tcp# output error: %d\n", error);
  }

  message.clear();
}

void TcpClientPrivate::buffer_response(const unsigned char *data, u16_t len) {
  if (last_response.size() > 3072) {
    printf("tcp# chunking last_response %d\n", last_response.size());
    last_response.clear();
  }

  printf("appending to response buffer %d\n", len);
  last_response += std::string(reinterpret_cast<const char *>(data), len);
}

TcpClient::TcpClient() : d(std::make_unique<TcpClientPrivate>()) {}

TcpClient::~TcpClient() {}

bool TcpClient::is_finished() const { return d->is_finished(); }

bool TcpClient::send_message(const ip_addr_t &ip, const int port,
                             std::string &&message) {
  return d->send_message(ip, port, std::move(message));
}

std::string TcpClient::get_last_response() { return d->get_last_response(); }
