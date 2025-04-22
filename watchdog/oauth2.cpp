#include "oauth2.h"
#include "base64.h"
#include "dateutils.h"
#include "rs256.h"
#include "stringutils.h"

#include <algorithm>

namespace {
std::optional<std::string> parse_string_between(std::string_view response,
                                                std::string_view a,
                                                std::string_view b1,
                                                std::string_view b2) {
  const auto start = response.find(a);
  if (start == std::string::npos) {
    return std::nullopt;
  }

  auto end = response.find(b1, start + a.length());
  if (end == std::string::npos) {
    end = response.find(b2, start + a.length());
    if (end == std::string::npos) {
      return std::nullopt;
    }
  }
  return std::string{response}.substr(start + a.length(),
                                      end - start - a.length());
}

std::string cleanup_base64(std::string str) {
  std::replace(str.begin(), str.end(), '+', '-');
  std::replace(str.begin(), str.end(), '/', '_');
  str.erase(std::remove(str.begin(), str.end(), '='), str.end());
  return str;
}

} // namespace

struct OAuth2Private {
  OAuth2Private(OAuth2::Config &&cfg) : cfg(std::move(cfg)) {}

  OAuth2::Config cfg;
};

OAuth2::OAuth2(Config &&cfg)
    : d(std::make_unique<OAuth2Private>(std::move(cfg))) {}

OAuth2::~OAuth2() {}

// JSON Web Token (JWT) Profile for OAuth 2.0 Client Authentication and
// Authorization Grants https://www.rfc-editor.org/rfc/rfc7523#section-2.1
HTTP::Request OAuth2::create_request() {
  std::string header = R"({
      "typ": "JWT",
      "alg": "RS256",
      "kid": ")" + d->cfg.private_key_id +
                       R"("
    })";

  const auto iat = DateUtils::unixnowseconds();
  const auto exp = iat + 3600ull;

  std::string payload = R"({
      "iat": )" + std::to_string(iat) +
                        R"(,
      "exp": )" + std::to_string(exp) +
                        R"(,
      "iss": ")" + d->cfg.client_email +
                        R"(",
      "aud": ")" + d->cfg.token_uri +
                        R"(",
      "scope": ")" + d->cfg.scope +
                        R"("
    })";

  std::string content =
      cleanup_base64(BASE64::base64(StringUtils::clean_whitespace(header))) +
      "." +
      cleanup_base64(BASE64::base64(StringUtils::clean_whitespace(payload)));
  std::string signature =
      cleanup_base64(BASE64::base64(RS256::sign(content, d->cfg.private_key)));
  std::string form_url_encoded =
      "assertion=" + content + "." + signature +
      "&grant_type=urn%3Aietf%3Aparams%3Aoauth%3Agrant-type%3Ajwt-bearer";

  return HTTP::Request{
      11, "POST", "/token",
      std::unordered_map<std::string, std::string>{
          {"Host", "oauth2.googleapis.com"},
          {"Connection", "close"},
          {"Content-Type", "application/x-www-form-urlencoded"}},
      form_url_encoded};
}

// {"access_token":"...","expires_in":3599,"token_type":"Bearer"}
std::optional<OAuth2::Response>
OAuth2::parse_response(const HTTP::Response &response) {
  const auto cleaned_response = StringUtils::clean_whitespace(response.body);
  if (const auto expires = parse_string_between(
          cleaned_response, R"("expires_in":)", R"(,)", "}");
      expires) {
    auto access_token = parse_string_between(
        cleaned_response, R"("access_token":")", R"(")", R"("})");
    if (access_token.has_value()) {
      return OAuth2::Response{access_token.value(), atoi(expires->c_str())};
    }
  }
  return std::nullopt;
}
