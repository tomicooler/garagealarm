#ifndef HTTP_H
#define HTTP_H

#include <ostream>
#include <string>
#include <unordered_map>

// VERY VERY DUMMY HTTP "IMPLEMENTATION"
// DO NOT USE IN YOUR PROJECT
namespace HTTP {

struct Request {
  int version;
  std::string method;
  std::string target;
  std::unordered_map<std::string, std::string> fields;
  std::string body;
};

std::ostream &operator<<(std::ostream &s, const Request &r);
std::string write(const Request &request);

struct Response {
  int version;
  int status;
  std::string reason;
  std::unordered_map<std::string, std::string> fields;
  std::string body;
};

Response read(std::string_view data);

} // namespace HTTP

#endif // HTTP_H
