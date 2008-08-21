#ifndef _GRAAL_CORE_HELPER
#define _GRAAL_CORE_HELPER

#include <string>
#include <vector>

namespace Graal {
  // converts csv strings ("foo","bar") to strings ("foo\nbar")
  std::vector<std::string> csv_to_array(const std::string& str);
  std::string csv_to_string(const std::string& str);
  std::string string_to_csv(const std::string& str);

  void str_toupper(std::string &str);

  void str_tolower(std::string &str);

  struct inspect_str {
    inspect_str(const std::string& str);

    const std::string& s;
  };

  std::ostream& operator<<(std::ostream& out, const inspect_str& s);
}

#endif
