#ifndef GRAAL_LEVEL_EDITOR_HELPER_HPP_
#define GRAAL_LEVEL_EDITOR_HELPER_HPP_

#include <string>
#include <sstream>
#ifdef DEBUG
  #include <iostream>
#endif

namespace Graal {
  namespace helper {
    static const std::string BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    int get_tile_x(int index);
    int get_tile_y(int index);
    int get_tile_index(int x, int y);

    std::string strip(const std::string& str);
    
    template<typename T>
    bool parse(const std::string& str, T& output) {
      std::istringstream ss(str);
      T value;
      ss >> value;
      if (ss.fail())
      return false;

      output = value;
      return true;
    }

    template<typename T>
    T bound_by(T val, T lower_bound, T upper_bound) {
      return std::max(lower_bound, std::min(upper_bound, val));
    }

    std::size_t parse_base64(const std::string& str);
    std::string format_base64(std::size_t num, std::size_t len = 2);
  }
}

#endif

