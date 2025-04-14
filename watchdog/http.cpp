#include "http.h"

#include <sstream>

namespace HTTP {
std::string version_string(const Request &r) {
  if (r.version == 11) {
    return std::string{"HTTP/1.1"};
  }
  return std::string{"HTTP/1"};
}
} // namespace HTTP

std::ostream &HTTP::operator<<(std::ostream &s, const Request &r) {
  s << r.method << " " << r.target << " " << version_string(r) << "\r\n";
  for (const auto &m : r.fields) {
    s << m.first << ": " << m.second << "\r\n";
  }
  s << "Content-Length: " << r.body.size() << "\r\n\r\n";
  s << r.body;
  return s;
}

std::string HTTP::write(const Request &request) {
  std::ostringstream oss;
  oss << request;
  return oss.str();
}

HTTP::Response HTTP::read(std::string_view data) {
  HTTP::Response resp;
  auto newline = data.find("\r\n");
  if (newline == std::string::npos) {
    return resp;
  }

  const auto first_line = data.substr(0, newline);
  auto s0 = first_line.find(" ");
  if (s0 == std::string::npos) {
    return resp;
  }

  const auto version = first_line.substr(0, s0);
  resp.version = version == std::string{"HTTP/1.1"} ? 11 : 1;

  const auto s1 = first_line.find(" ", s0 + 1);
  if (s1 == std::string::npos) {
    return resp;
  }

  const auto status = first_line.substr(s0 + 1, s1 - s0 - 1);
  resp.status = atoi(std::string{status}.c_str());

  resp.reason = first_line.substr(s1 + 1);

  while (true) {
    auto next_new_line = data.find("\r\n", newline + 2);
    if (next_new_line == std::string::npos) {
      return resp;
    }
    const auto field = data.substr(newline + 2, next_new_line - newline - 2);
    if (newline + 2 == next_new_line) {
      resp.body = data.substr(next_new_line + 2);
      break;
    }

    const auto separator = field.find(":");
    if (separator == std::string::npos) {
      return resp;
    }
    resp.fields[std::string{field.substr(0, separator)}] =
        field.substr(separator + 2);

    newline = next_new_line;
  }

  return resp;
}
