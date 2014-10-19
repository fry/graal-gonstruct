#ifndef PREFERENCES_HPP_
#define PREFERENCES_HPP_

#include <boost/filesystem/path.hpp>
#include <map>
#include <string>

namespace Graal {
  class preferences {
  public:
    typedef std::map<std::string, std::string> value_map_type;
  
    void load(const boost::filesystem::path& path);
    void save(const boost::filesystem::path& path);

    virtual ~preferences() {}
  protected:
    // Virtual functions for the derived class to parse & write the entries
    virtual void serialize() {}    // write
    virtual void deserialize() {}  // parse

    value_map_type m_values;
  };
}

#endif
