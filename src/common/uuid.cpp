#include "uuid.h"
#include <algorithm>
#include <cassert>
#include <iomanip>
#include <ios>
#include <random>
#include <sstream>
#include <string>

uuid::uuid(const std::string &uuid) {
  std::stringstream stream(uuid);

  unsigned long input = 0;
  stream >> std::hex >> input;
  octets[3] = input & 0xff;
  input     >>= 8;
  octets[2] = input & 0xff;
  input     >>= 8;
  octets[1] = input & 0xff;
  input     >>= 8;
  octets[0] = input & 0xff;

  char character = 0;
  stream >> character;
  assert(character == '-');

  stream >> std::hex >> input;
  octets[5] = input & 0xff;
  input     >>= 8;
  octets[4] = input & 0xff;

  stream >> character;
  assert(character == '-');

  stream >> std::hex >> input;
  octets[7] = input & 0xff;
  input     >>= 8;
  octets[6] = input & 0xff;

  stream >> character;
  assert(character == '-');

  stream >> std::hex >> input;
  octets[9] = input & 0xff;
  input     >>= 8;
  octets[8] = input & 0xff;

  stream >> character;
  assert(character == '-');

  stream >> std::hex >> input;
  octets[15] = input & 0xff;
  input      >>= 8;
  octets[14] = input & 0xff;
  input      >>= 8;
  octets[13] = input & 0xff;
  input      >>= 8;
  octets[12] = input & 0xff;
  input      >>= 8;
  octets[11] = input & 0xff;
  input      >>= 8;
  octets[10] = input & 0xff;
}

auto uuid::generateRandomUuid() -> uuid {
  static std::random_device                           randomDevice;
  static std::mt19937                                 generator(randomDevice());
  static std::uniform_int_distribution<unsigned char> distribution;

  uuid result;
  std::generate(result.octets.begin(), result.octets.end(), []() {
    return distribution(generator);
  });

  result.octets[6] &= 0x0f;
  result.octets[6] |= 0x40;

  result.octets[8] &= 0b00111111;
  result.octets[8] |= 0b10000000;

  return result;
}

auto uuid::ToString() const -> std::string {
  std::stringstream stream;

  int index = 0;
  for (auto octetIterator = octets.begin(); octetIterator != octets.end(); ++octetIterator, ++index) {
    stream << std::hex << std::setfill('0') << std::setw(2) << +*octetIterator;
    if (index == 3 || index == 5 || index == 7 || index == 9) {
      stream << '-';
    }
  }
  return stream.str();
}
