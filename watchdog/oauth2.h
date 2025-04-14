#ifndef OAUTH2_H
#define OAUTH2_H

#include <memory>
#include <optional>
#include <string>

#include "http.h"

struct OAuth2Private;

class OAuth2 {
public:
  struct Config {
    std::string client_email;
    std::string private_key_id;
    std::string private_key;
    std::string project_id;
    std::string token_uri;
    std::string scope;
  };

  struct Response {
    std::string access_token;
    int expires_in;
  };

  explicit OAuth2(Config &&cfg);
  ~OAuth2();

  HTTP::Request create_request();
  static std::optional<Response> parse_response(const HTTP::Response &response);

private:
  std::unique_ptr<OAuth2Private> d;
};

#endif // OAUTH2_H
