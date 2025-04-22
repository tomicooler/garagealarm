#include "base64.h"

#include "mbedtls/base64.h"

std::string BASE64::base64(const std::string &data) {
  std::string out;
  const auto n = data.size() / 3 + (data.size() % 3 != 0);
  out.resize((n * 4) + 1);
  size_t encoded_length = 0;
  if (const auto ret = mbedtls_base64_encode(
          reinterpret_cast<unsigned char *>(out.data()), out.size(),
          &encoded_length,
          reinterpret_cast<const unsigned char *>(data.c_str()), data.size());
      ret != 0) {
    printf("!mbedtls_base64_encode %d\n", ret);
    return {};
  }
  out.resize(encoded_length);
  return out;
}
