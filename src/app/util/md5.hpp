#pragma once

#include <openssl/evp.h>
#include <stdexcept>
#include <string>

static std::string calculateMD5(const char *input, int size) {
  unsigned char md5Hash[EVP_MAX_MD_SIZE];
  unsigned int md5HashLength;

  std::string output;

  // Create a message digest context
  auto mdctx = EVP_MD_CTX_new();

  // Initialize the message digest context
  if (mdctx == nullptr || !EVP_DigestInit_ex2(mdctx, EVP_md5(), nullptr)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to initialize hash context");
  }

  // Add data to the message digest context
  if (!EVP_DigestUpdate(mdctx, input, size)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to update hash context");
  }

  // Finalize the hash computation
  if (!EVP_DigestFinal_ex(mdctx, md5Hash, &md5HashLength)) {
    EVP_MD_CTX_free(mdctx);
    throw std::runtime_error("Failed to finalize hash context");
  }

  // Clean up the message digest context
  EVP_MD_CTX_free(mdctx);

  output.resize(md5HashLength * 2);
  for (unsigned int i = 0; i < md5HashLength; ++i)
    std::sprintf(&output[i * 2], "%02x", md5Hash[i]);

  return output;
}