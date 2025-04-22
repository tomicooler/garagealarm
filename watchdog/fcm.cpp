#include "fcm.h"

#include "dateutils.h"
#include "stringutils.h"

namespace {
std::string action_to_string(Action action) {
  switch (action) {
  case Action::DoorOpen:
    return std::string{"open"};
  case Action::DoorClose:
    return std::string{"close"};
  case Action::IvTopMismatch:
    return std::string{"iv_top_mismatch"};
  case Action::IvBottomOld:
    return std::string{"iv_bottom_old"};
  }
  return std::string{};
}
} // namespace

struct FCMPrivate {
  FCMPrivate(std::string &&project_id, std::string &&device_token)
      : project_id(std::move(project_id)),
        device_token(std::move(device_token)) {}

  std::string project_id;
  std::string device_token;
};

FCM::FCM(std::string &&project_id, std::string &&device_id)
    : d(std::make_unique<FCMPrivate>(std::move(project_id),
                                     std::move(device_id))) {}

FCM::~FCM() {}

HTTP::Request FCM::create_request(Action action, const std::string &bearer) {
  auto content =
      StringUtils::clean_whitespace(R"({
      "message": {
        "token": ")" + d->device_token +
                                    R"(",
        "android":{
          "priority": "high"
        },
        "data": {
            "timestamp": ")" +
                                    std::to_string(DateUtils::unixnowseconds() *
                                                   1000) +
                                    R"(",
            "action": ")" + action_to_string(action) +
                                    R"("
        }
      }
    })");

  return HTTP::Request{11, "POST",
                       "/v1/projects/" + d->project_id + "/messages:send",
                       std::unordered_map<std::string, std::string>{
                           {"Host", "fcm.googleapis.com"},
                           {"Connection", "close"},
                           {"Content-Type", "application/json"},
                           {"Authorization", "Bearer " + bearer}},
                       content};
}
