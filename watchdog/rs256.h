#ifndef RS256_H
#define RS256_H

#include <string>

namespace RS256 {

std::string sign(const std::string &data, const std::string &private_key);

}

#endif // RS256_H
