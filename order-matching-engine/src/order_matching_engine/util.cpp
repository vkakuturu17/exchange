#include "util.h"

namespace Exchange {

// Hash function
uint64_t StringHash(std::string str) {
  std::hash<std::string> hash_fn;
  return hash_fn(str);
}

std::string AddCharArray(char* arr, size_t len, char* arr2, size_t len2) {
  std::string str(arr, len);
  std::string str2(arr2, len2);
  return str + str2;
}

std::string AddCharArray(const char* arr, size_t len, const char* arr2, size_t len2) {
  std::string str(arr, len);
  std::string str2(arr2, len2);
  return str + str2;
}

void CopyStringToCharArray(unsigned char buffer[], std::string str, size_t len) {
  size_t str_len = str.length();
  size_t i = 0;
  for (; i < len && i < str_len; i++) {
    buffer[i] = str[i];
  }
  if (str_len < len) {
    buffer[++i] = '\0';
  }
}

void CopyStringToCharArray(char buffer[], std::string str, size_t len) {
  size_t str_len = str.length();
  size_t i = 0; 
  for (; i < len && i < str_len; i++) {
    buffer[i] = str[i];
  }
  if (str_len < len) {
    buffer[i] = '\0';
  }
}

void CopyStringToCharArray(char buffer[], std::string str, size_t len, size_t offset) {
  size_t str_len = str.length();
  size_t i = 0; 
  for (; i < len && i < str_len; i++) {
    buffer[i + offset] = str[i];
  }
  if (str_len < len) {
    buffer[++i + offset] = '\0';
  }
}

std::string StringFromArray(const char* arr, size_t len) {
  if (arr[len] != '\0') return std::string(arr, len);
  else {
    size_t i = 0;
    for (; i < len; i++) {
      if (arr[i] == '\0') break;
    }
    return std::string(arr, i);
  }
}

} // namespace Exchange