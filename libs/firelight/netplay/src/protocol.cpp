#include <firelight/netplay/protocol.hpp>

#include <random>

namespace firelight::netplay {

std::string generateJoinCode() {
  // Crockford base32: no I, L, O, U — unambiguous to read aloud
  static constexpr char ALPHABET[] = "0123456789ABCDEFGHJKMNPQRSTVWXYZ";
  std::random_device device;
  std::uniform_int_distribution<int> pick(0, 31);

  std::string code = "FL-";
  for (int i = 0; i < 8; ++i) {
    if (i == 4) {
      code += '-';
    }
    code += ALPHABET[pick(device)];
  }
  return code;
}

} // namespace firelight::netplay
