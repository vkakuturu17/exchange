#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>
#include <string.h>
#include <stdio.h>
#include <byteswap.h>

using std::cout;
using std::endl;
using std::unique_ptr;
using std::vector;

#include "order_matching_engine/util.h"
using Exchange::AddCharArray;

int main() {
  unsigned char buffer[32];
  char c = 'A';
  uint64_t time = 1234567890;
  time = bswap_64(time);

  memcpy(buffer, &c, 1);
  memcpy(buffer + 1, &time, 8);
  buffer[31] = '\0';

  for (int i = 0; i < 32; i++) {
    printf("%x ", buffer[i]);
  }

  char c2;
  uint64_t time2;
  memcpy(&c2, buffer, 1);
  memcpy(&time2, buffer + 1, 8);
  time2 = bswap_64(time2);
  printf("%c\n", c2);
  printf("%ld\n", time2);

  char buffer2[5] = {'H', 'e', 'l', 'l', 'o'};
  std::string str(buffer2 + '\0');
  std::cout << str << std::endl;

  char buffer3[5] = {'W', 'o', 'r', 'l', 'd'};
  std::string str2 = AddCharArray(buffer2, 5, buffer3, 5);
  std::cout << str2 << std::endl;

  return 0;
}