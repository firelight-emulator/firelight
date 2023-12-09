//
// Created by alexs on 11/25/2023.
//

#include "yay_0_codec.hpp"
#include <cstdio>
#include <cstring>

namespace FL::Patching {

struct Header {
  uint32_t magic; // should just be ASCII 'Yay0'
  uint32_t uncompressedLength;
  uint32_t opsAddr;  // Address of beginning of opcodes
  uint32_t dataAddr; // Address of beginning of compressed data
};

// algorithm from
// https://github.com/pmret/papermario/blob/main/tools/splat/util/n64/Yay0decompress.c
std::vector<uint8_t> Yay0Codec::decompress(uint8_t *data) {

  // Fun little test that checks if the system is big or little endian.
  unsigned int num = 1;
  char *test = (char *)&num;

  bool sysBigEndian = (*test != 1);
  // TODO: really just need to check

  // Check if the beginning is 'Yay0' or '0yaY' lol
  bool dataBigEndian = data[0] == 'Y';

  Header header{};

  header.magic = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
  header.uncompressedLength = ((uint8_t)data[4]) << 24 |
                              ((uint8_t)data[5]) << 16 |
                              ((uint8_t)data[6]) << 8 | ((uint8_t)data[7]);
  header.opsAddr = data[8] << 24 | data[9] << 16 | data[10] << 8 | data[11];
  header.dataAddr = data[12] << 24 | data[13] << 16 | data[14] << 8 | data[15];

  printf("magic: %0x\n", header.magic);
  printf("uncompressed: %u\n", header.uncompressedLength);
  printf("opPtr: %0x\n", header.opsAddr);
  printf("dataPtr: %0x\n", header.dataAddr);

  auto dest = new uint8_t[header.uncompressedLength];
  auto dstPtr = dest;

  auto srcPtr = data;

  uint8_t byte = 0, mask = 0;
  uint8_t *ctrl, *ops, *uncData;
  uint16_t copy, op;
  uint32_t written = 0;

  ctrl = srcPtr + sizeof(header);
  ops = srcPtr + header.opsAddr;
  uncData = srcPtr + header.dataAddr;

  while (written < header.uncompressedLength) {
    if ((mask >>= 1) == 0) {
      byte = *ctrl++;
      mask = 0x80;
    }

    if (byte & mask) {
      *dstPtr++ = *uncData++;
      written++;
    } else {
      op = dataBigEndian ? (ops[0] << 8) | ops[1] : (ops[1] << 8) | ops[0];
      ops += 2;

      written += copy = (op >> 12) ? (2 + (op >> 12)) : (18 + *uncData++);

      while (copy--) {
        *dstPtr = dstPtr[-(op & 0xfff) - 1];
        dstPtr++;
      }
    }
  }

  std::vector<uint8_t> result(header.uncompressedLength);
  memcpy(result.data(), dest, header.uncompressedLength);

  return result;
}
} // namespace FL::Patching
