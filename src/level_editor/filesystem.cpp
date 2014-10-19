#include "filesystem.hpp"
#include <iostream>
#include <set>
#include <core/helper.h>
#include <fstream>
#include "helper.hpp"

using namespace Graal;
using namespace Graal::helper;
using namespace Graal::level_editor;

filesystem::filesystem(preferences& _prefs): m_preferences(_prefs) {
}

bool filesystem::valid_dir() {
  std::string graal_dir = m_preferences.graal_dir;
  return boost::filesystem::exists(graal_dir) && 
         boost::filesystem::is_directory(graal_dir);
}

void filesystem::update_cache() try {
  // clear first. if the iterator blows up right away, at least we're not
  // left with a stale cache.
  m_cache.clear();

  boost::filesystem::recursive_directory_iterator iter(m_preferences.graal_dir), end;
  while (iter != end) {
    const boost::filesystem::path& path = iter->path();
    m_cache[path.filename().string()] = path;

    // can't use iter.increment(ec) here:
    // https://svn.boost.org/trac/boost/ticket/5403
    try {
      ++iter;
    } catch (const boost::filesystem::filesystem_error&) {
      // something went wrong. let's assume it was because we couldn't recurse,
      // so let's not do that and increment again.
      iter.no_push();
      // if this fails again, let it blow up.
      ++iter;
    }
  }
} catch (boost::filesystem::filesystem_error&) {
  // TODO: report error?
}

bool filesystem::get_path(const std::string& file_name,
                                        boost::filesystem::path& path_found) {
  // If the requested file name is a exact path and exists, use it instead
  boost::filesystem::path file_path(file_name);
  if (file_path.has_root_directory() && boost::filesystem::exists(file_name)) {
    path_found = file_path;
    return true;
  }

  if (!valid_dir()) {
    return false;
  }

  filesystem::cache_type::iterator fiter, fend;
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

bool filesystem::update_cache_from_graal() {
  if (!valid_dir())
    return false;

  boost::filesystem::path graal_dir(m_preferences.graal_dir);
  boost::filesystem::path cache_file_name = graal_dir / "FILENAMECACHE.txt";

  if (!boost::filesystem::exists(cache_file_name))
    return false;

  std::ifstream cache_file(cache_file_name.string().c_str());

  if (!cache_file.good())
    return false;

  while (!cache_file.eof()) {
    std::string line = read_line(cache_file);
    std::vector<std::string> tokens = csv_to_array(line);
    if (!tokens.empty()) {
      boost::filesystem::path file(tokens[0]);
      m_cache[file.filename().string()] = graal_dir / file;
    } 
  }

  return true;
}

/*void level_editor::filesystem::load(const boost::filesystem::path& path) {}
void level_editor::filesystem::save(const boost::filesystem::path& path) {}*/
