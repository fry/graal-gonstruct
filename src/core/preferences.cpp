#include "preferences.h"
#include <boost/filesystem.hpp>
#include <fstream>

using namespace Graal;

preferences::~preferences() {}

void preferences::load(const boost::filesystem::path& path) {
  if (!boost::filesystem::exists(path))
    return;

  // Not using boost::filesystem::ifstream because it surprisingly fails again!
  std::ifstream stream(path.string().c_str());

  if (stream.bad()) {
    throw std::runtime_error("Can't read " + path.string());
  }

  m_values.clear();

  std::string line, key, value;
  while (stream.good()) {
    std::getline(stream, line);
    
    std::size_t equal = line.find_first_of("=");
    if (equal == std::string::npos)
      continue;

    std::size_t pos = line.find_first_of(" ");
    if (pos == std::string::npos)
      continue;
    key = line.substr(0, pos);

    pos = line.find_first_not_of(" ", equal + 1);
    if (pos == std::string::npos)
      continue;
    value = line.substr(pos);

    m_values[key] = value;
  }

  stream.close();

  deserialize();
}

void preferences::save(const boost::filesystem::path& path) {
  std::ofstream stream(path.string().c_str());

  if (stream.bad()) {
    throw std::runtime_error("Can't write " + path.string());
  }

  m_values.clear();
  serialize();
  value_map_type::iterator iter, end;
  end = m_values.end();

  for (iter = m_values.begin(); iter != end; iter ++) {
    stream << iter->first << " = " << iter-> second << std::endl;
  }

  stream.close();

}
