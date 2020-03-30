#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/support/lambda.hpp>
#include <boost/range/algorithm/find.hpp>
#include <iomanip>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace bm = boost::bimaps;

std::string GetCrc32(const std::string& block) {
  boost::crc_32_type crc_32;
  crc_32.process_bytes(block.data(), block.length());
  std::ostringstream buf;
  buf << std::hex << std::uppercase << crc_32.checksum();
  return buf.str();
}
std::string GetSha1(const std::string& block) {
  boost::uuids::detail::sha1 sha1;
  sha1.process_bytes(block.data(), block.length());
  unsigned int hash[5] = {0};
  sha1.get_digest(hash);
  std::ostringstream buf;
  for(int i = 0; i < 5; ++i)
      buf << std::hex << std::setfill('0') << std::setw(8) << hash[i];

  return buf.str(); 
}
std::string GetHash(const std::string& block, bool hash_default) {
  if (hash_default)
    return GetCrc32(block);
  else
    return GetSha1(block);
}
void GetPath(fs::path p, boost::bimap<fs::path, boost::bimaps::multiset_of<std::string>> &files)
{
  if (fs::is_regular_file(p))
    files.insert({p, std::to_string(fs::file_size(p))});
  else if (fs::is_directory(p))
  {
    for (auto x : fs::directory_iterator(p))
      GetPath(x, files);
  }
  else
    throw std::runtime_error(p.string() + " exists, but is not a regular file or directory");
}
void read_file(fs::path fname, size_t &pos, std::string &hash, size_t &block_size, bool hash_default)
{
  fs::fstream file;
  file.rdbuf()->pubsetbuf(nullptr, 0);
  file.open(fname);
  if (!file.is_open())
    throw std::runtime_error("error open " + fname.string());
  file.seekg(pos);
  std::string buf;
  buf.resize(block_size, '\0');
  file.read(&buf[0], block_size);
  hash = GetHash(buf, hash_default);
  file.close();
}