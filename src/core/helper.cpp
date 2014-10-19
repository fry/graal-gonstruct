#include "helper.h"
#include "csvparser.h"

#include <cctype>
#include <algorithm>
#include <string>

#include <iostream>
#include <iomanip>

namespace Graal {
  void str_toupper(std::string &str) {
    std::transform(str.begin(), 
                   str.end(), 
                   str.begin(),
                   static_cast<int(*)(int)>(std::toupper));
  }

  void str_tolower(std::string &str) {
    std::transform(str.begin(), 
                   str.end(), 
                   str.begin(),
                   static_cast<int(*)(int)>(std::tolower));
  }
  // TODO: UGLY
  std::vector<std::string> csv_to_array(const std::string& str) {
    std::vector<std::string> values;
    CSVParser parser;
    parser << str;

    std::string temp;
    while (!parser.eof()) {
      parser >> temp;
      values.push_back(temp);
    }

    return values;
  }

  std::string csv_to_string(const std::string& str) {
    std::string result;
    CSVParser parser;
    parser << str;

    std::string temp;
    while (!parser.eof()) {
      parser >> temp;
      result += temp;
      result += "\n";
    }
    return result;
  }

  std::string string_to_csv(const std::string& str) {
    std::string result;
 
    std::string::size_type pos, oldpos, pos2;
    pos = oldpos = pos2 = 0;
    std::string subs;

    std::string source = str;
    // replace " with ""
    while (true) {
      oldpos = pos2;
      pos2 = source.find("\"", oldpos);
      if (pos2 == std::string::npos)
        break;
      source.erase(pos2, 1);
      source.insert(pos2, "\"\"");
      // skip old + new character (\")
      pos2 += 2;
    }

    std::string::size_type size = source.length();
    while (pos < size) {
      pos = source.find("\n");
      subs = source.substr(0, pos);
      source = source.substr(pos + 1);
      if (source.find(" ", 0) != std::string::npos) {
        result += "\"";
        result += subs;
        result += "\"";
      } else
        result += subs;

      if (pos != std::string::npos)
        result += ",";
    }


    return result;
  }

  inspect_str::inspect_str(const std::string& str) : s(str) {}

  std::ostream& operator<<(std::ostream& out, const inspect_str& s) {
    std::ios::fmtflags old_flags
      = out.setf(std::ios::hex, std::ios::basefield);
    std::ostream::char_type old_fill = out.fill('0');
    for (std::string::const_iterator i = s.s.begin(); i != s.s.end(); ++i) {
      char c = *i;
      if (std::isprint(c))
        out.put(c);
      else
        out << "\\x" << std::setw(2)
            << static_cast<unsigned int>(c);
    }
    out << std::flush;
    out.setf(old_flags, std::ios::basefield);
    out.fill(old_fill);

    return out;
  }
}
