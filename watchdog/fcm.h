#ifndef FCM_H
#define FCM_H

#include <memory>

#include "action.h"
#include "http.h"

struct FCMPrivate;

class FCM {
public:
  explicit FCM(std::string &&project_id, std::string &&device_token);
  ~FCM();

  HTTP::Request create_request(Action action, const std::string &bearer);

private:
  std::unique_ptr<FCMPrivate> d;
};

#endif // FCM_H
