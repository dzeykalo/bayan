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

int main(int argc, const char* argv[])
{
  po::options_description desc("options");
  po::variables_map vm;
  fs::path input_path;
  size_t block_size;
  bool hash_default = true;
  boost::bimap<std::string, boost::bimaps::multiset_of<size_t>> in_files;

  desc.add_options()
          ("help,h", "show help")
          ("input,i", po::value<std::vector<std::string>>(), "name input files or directory")
          ("block-size,b",  po::value<size_t>(&block_size)->default_value(5), "block size in bytes. Default 5")
          ("hash,s", po::value<std::string>()->default_value("crc32"), "Hash algorithm: sha1 or crc32. Default crc32")
          ;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("block-size")){
      if (block_size < 1 || block_size > 100){
        std::cout << "invalid block size: " <<  block_size << " set default (5) " << std::endl;
        block_size = 5;
      }
    }

    if (vm.count("hash")){
      std::string hash_logic = vm["hash"].as<std::string>();
      if (hash_logic == "sha1" || hash_logic == "SHA1")
        hash_default = false;
    }

    if (vm.count("input"))
    {
      auto input_files = vm["input"].as<std::vector<std::string>>();
      // TO DO checking file or path
      
      for(auto &it : input_files)
      {
        in_files.insert({it, fs::file_size(it)});
      }
      for (auto &it : in_files){
        if (in_files.right.count(it.right) < 2)
          in_files.erase(it);
      }
      if (in_files.size() < 2){
        std::cout << "no duplicates due to size " << std::endl;
        return EXIT_SUCCESS;
      }

      // if (fs::is_regular_file(p))
      //   std::cout << fs::file_size(s) << std::endl;
      // fs::path p(s);
    }

    bool check = true;
    size_t pos = 0;

    while (check)
    {
      boost::bimap<std::string, boost::bimaps::multiset_of<std::string>> buffer_hash;
      for (auto &it : in_files){
        
        fs::ifstream file;
        file.open(it.left, std::ios::binary);
        if (!file.is_open()){
          std::cout << "error open "<< it.left << std::endl;
          return EXIT_FAILURE;
        }
        file.seekg(pos);
        std::string buf;
        buf.resize(block_size, '\0');
        if (!file.read(&buf[0], block_size))
        {
          file.close();
          std::cout << "duplicates = " << in_files.size() << std::endl;
          return EXIT_SUCCESS;
        }
        buffer_hash.insert({it.left, GetHash(buf, hash_default)});
        
        file.close();
      }
      auto i = in_files.begin();
      for (auto &it : buffer_hash){
        if (buffer_hash.right.count(it.right) < 2)
          in_files.erase(i);
        else
          i++;
      }
      if (in_files.size() < 2){
        std::cout << "no duplicates = " << in_files.size() << std::endl;
        check = false;
      }
      pos += block_size;
    }

    
    return EXIT_SUCCESS;
  }
  catch ( std::exception &e )
  {
    std::cerr << e.what() << std::endl;
  }
  catch (...)
  {
    std::cerr << "everything went completely wrong" << std::endl;
  }
  return EXIT_FAILURE;
  
}