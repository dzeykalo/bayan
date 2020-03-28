#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/sha1.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>
#include <boost/bimap/support/lambda.hpp>
#include <iomanip>
#include <string>
#include <vector>

namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace fn = boost::fusion;
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
void read_file(fs::path fname, size_t &pos, boost::bimap<fs::path, bm::multiset_of<std::string>> &buffer_hash, size_t &block_size, bool hash_default)
{
  fs::ifstream file;
  file.rdbuf()->pubsetbuf(nullptr, 0);
  file.open(fname, std::ios::binary);
  if (!file.is_open())
    throw "error open " + fname.string();

  file.seekg(pos);
  std::string buf;
  buf.resize(block_size, '\0');
  !file.read(&buf[0], block_size);
  buffer_hash.insert({fname, GetHash(buf, hash_default)});
  
  file.close();
}