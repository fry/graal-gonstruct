#include "helper.hpp"
#include <stdexcept>

using namespace Graal;

std::string helper::strip(const std::string& str, const char* ws) {
  std::string::size_type first, last, length;
  first = str.find_first_not_of(ws);
  last = str.find_last_not_of(ws);
  
  if (first == std::string::npos)
    first = 0;

  length = last - first + 1;

  return str.substr(first, length);
}

std::size_t helper::parse_base64(const std::string& str) {
  std::size_t num = 0;
  for (std::size_t i = 0; i < str.length(); ++i) {
    std::size_t pos = BASE64.find(str[i]);
    if (pos == std::string::npos) {
      throw std::runtime_error("BASE64: invalid format");
    }
    num += static_cast<int>(pos) << (str.length() - i - 1) * 6;
  }
  return num;
}

std::string helper::format_base64(std::size_t num, std::size_t len) {
  std::string str;
  for (std::size_t i = 0; i < len; ++i) {
    int index = (num >> (len - i - 1) * 6) & 0x3F; // 6 bit per character
    str += BASE64[index];
  }
  return str;
}
