#include "stringutils.h"

#include <algorithm>

namespace StringUtils {
std::string clean_whitespace(std::string str) {
  str.erase(std::remove_if(str.begin(), str.end(),
                           [](unsigned char x) { return std::isspace(x); }),
            str.end());
  return str;
}
} // namespace StringUtils
