#include "lib.h"

int main(int argc, const char* argv[])
{
  po::options_description desc("options");
  po::variables_map vm;
  fs::path input_path;
  const size_t max_block_size = 100;
  size_t block_size;
  bool hash_default = true;
  boost::bimap<fs::path, boost::bimaps::multiset_of<std::string>> files;

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

    if (vm.count("help")){
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

    if (vm.count("block-size")){
      if (block_size < 1 || block_size > max_block_size){
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
      
      for(auto &it : input_files)
        GetPath(it, files);

      for (auto &it : files){
        if (files.right.count(it.right) < 2)
          files.erase(it);
      }
      if (files.size() < 2)
        return EXIT_SUCCESS;
    }

    bool check = false;
    size_t pos = 0;
    
    do {
      check = false;
      for (auto it = files.left.begin(); it != files.left.end(); it++){
        if (fs::file_size(it->first) > pos){
          std::string hash;
          read_file(it->first, pos, hash, block_size, hash_default);
          files.left.modify_data(it, boost::bimaps::_data = it->second + hash);
          check = true;
        }
      }
      for (auto &it : files){
        if (files.right.count(it.right) < 2)
          files.erase(it);
      }
      if (files.size() < 2)
        return EXIT_SUCCESS;
      pos += block_size;

    } while(check);

    std::cout << std::endl;
    std::vector<std::string> v;
    for(auto &it : files){
      auto result = boost::find(v, it.right);
      if (result == v.end()){
        auto p = files.right.equal_range(it.right);
        for (auto &i = p.first; i != p.second; i++)
          std::cout << fs::system_complete(i->second) << std::endl;
        std::cout << std::endl;
        v.push_back(it.right);
      }      
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