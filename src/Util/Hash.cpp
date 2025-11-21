#include "Hash.h"

unique_ptr<byte[]> Hash::sha256(byte* payload) {
  mbedtls_sha256_context ctx;
  unique_ptr<byte[]> result(new byte[32]);

  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, payload, sizeof(payload));
  mbedtls_sha256_finish(&ctx, result.get());
  mbedtls_sha256_free(&ctx);

  return move(result);
}
