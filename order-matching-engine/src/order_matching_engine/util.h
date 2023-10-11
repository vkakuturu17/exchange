#pragma once
#include <string>
#include <functional>


namespace Exchange {

// Hash function
uint64_t StringHash(std::string str);

std::string AddCharArray(char* arr, size_t len, char* arr2, size_t len2);
std::string AddCharArray(const char* arr, size_t len, const char* arr2, size_t len2);
void CopyStringToCharArray(unsigned char buffer[], std::string str, size_t len);
void CopyStringToCharArray(char buffer[], std::string str, size_t len);
std::string StringFromArray(const char* arr, size_t len);


} // namespace Exchange