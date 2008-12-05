#include "filesystem.hpp"
#include <iostream>
#include <set>

using namespace Graal;

namespace {
  struct equivalent {
    equivalent(const boost::filesystem::path& p) : path(p) {}

    bool operator()(const boost::filesystem::path& p) {
      return boost::filesystem::equivalent(path, p);
    }

    const boost::filesystem::path& path;
  };
}

level_editor::filesystem::filesystem(preferences& _prefs): m_preferences(_prefs) {
}

void level_editor::filesystem::update_cache() {
  if (!boost::filesystem::is_directory(m_preferences.graal_dir))
    return;
  std::set<boost::filesystem::path> visited;

  boost::filesystem::recursive_directory_iterator iter(m_preferences.graal_dir), end;
  m_cache.clear();
  for (;iter != end; iter++) {
    const boost::filesystem::path& path = iter->path();
    const boost::filesystem::file_status& status = iter->status();
    if (status.type() == boost::filesystem::directory_file) {
      if (std::find_if(visited.begin(), visited.end(), equivalent(path))
          == visited.end()) {
        visited.insert(path);
      } else {
        iter.no_push();
      }
    } else {
      m_cache[iter->path().leaf()] = iter->path();
    }
  }
}

bool level_editor::filesystem::get_path(const std::string& file_name,
                                        boost::filesystem::path& path_found) {

  if (!boost::filesystem::exists(m_preferences.graal_dir)) {
    return false;
  }

  cache_type::iterator fiter, fend;
  fend = m_cache.end();
  fiter = m_cache.find(file_name);
  if (fiter != fend) {
    path_found = fiter->second;
    return true;
  }

  // TODO: too slow, figure out another way to refresh cache
  /*boost::filesystem::recursive_directory_iterator iter(m_preferences.graal_dir), end;
  for (;iter != end; iter++) {
    if (boost::filesystem::is_regular(iter->status()) && iter->path().leaf() == file_name) {
      path_found = iter->path();
      return true;
    }
  }*/

  return false;
}

