#include "rs256.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/pk.h"

#include <cstring>

template <typename T> class wrap final {
public:
  using value_type = T;
  typedef void (*f_init_t)(value_type *);
  typedef void (*f_free_t)(value_type *);

  wrap(f_init_t init, f_free_t free) : m_struct(), m_deleter(free) {
    init(&m_struct);
  }
  ~wrap() { m_deleter(&m_struct); }

  wrap(const wrap &) = delete;
  wrap &operator=(const wrap &) = delete;
  wrap(wrap &&) = delete;
  wrap &operator=(wrap &&) = delete;

  value_type *operator&() { return &m_struct; }
  const value_type *operator&() const { return &m_struct; }

  value_type *operator->() { return &m_struct; }
  const value_type *operator->() const { return &m_struct; }

  value_type &operator*() { return m_struct; }
  const value_type &operator*() const { return m_struct; }

private:
  value_type m_struct;
  f_free_t m_deleter;
};

std::string RS256::sign(const std::string &data,
                        const std::string &private_key) {
  wrap<mbedtls_entropy_context> entropy(mbedtls_entropy_init,
                                        mbedtls_entropy_free);
  wrap<mbedtls_ctr_drbg_context> ctr_drbg(mbedtls_ctr_drbg_init,
                                          mbedtls_ctr_drbg_free);
  wrap<mbedtls_pk_context> pk(mbedtls_pk_init, mbedtls_pk_free);

  constexpr unsigned char pers[] = "mbedtls_pk_sign";
  if (const auto ret =
          mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, pers,
                                strlen(reinterpret_cast<const char *>(pers)));
      ret != 0) {
    printf("!mbedtls_ctr_drbg_seed %d\n", ret);
    return {};
  }

  if (const auto ret = mbedtls_pk_parse_key(
          &pk, reinterpret_cast<const unsigned char *>(private_key.c_str()),
          private_key.size(), NULL, 0);
      ret != 0) {
    printf("!mbedtls_pk_parse_key %d\n", ret);
    return {};
  }

  unsigned char hash[32];
  if (const auto ret =
          mbedtls_md(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                     reinterpret_cast<const unsigned char *>(data.c_str()),
                     data.size(), hash);
      ret != 0) {
    printf("!mbedtls_md %d\n", ret);
    return {};
  }

  std::string out;
  out.resize(MBEDTLS_PK_SIGNATURE_MAX_SIZE);
  size_t out_len = 0;
  if (const auto ret =
          mbedtls_pk_sign(&pk, MBEDTLS_MD_SHA256, hash, 0,
                          reinterpret_cast<unsigned char *>(out.data()),
                          &out_len, mbedtls_ctr_drbg_random, &ctr_drbg);
      ret != 0) {
    printf("!mbedtls_pk_sign %d\n", ret);
    return {};
  }
  out.resize(out_len);

  return out;
}
