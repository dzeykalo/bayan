#include "lib.h"

int main(int argc, const char* argv[])
{
  po::options_description desc("options");
  po::variables_map vm;
  fs::path input_path;
  const size_t max_block_size = 100;
  size_t block_size;
  bool hash_default = true;
  boost::bimap<fs::path, boost::bimaps::multiset_of<size_t>> files;

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
      {
        if (fs::is_regular_file(it))
          files.insert({it, fs::file_size(it)});
        else if (fs::is_directory(it))
        {
          for (auto &x : fs::directory_iterator(it))
            files.insert({x, fs::file_size(x)});
        }
        else
          throw std::runtime_error(it + " exists, but is not a regular file or directory");
      }

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
      boost::bimap<fs::path, bm::multiset_of<std::string>> buffer_hash;
      for (auto &it : files){
        if (it.right > pos){
          read_file(it.left, pos, buffer_hash, block_size, hash_default);
          check = true;
        }
      }
      auto i = files.begin();
      for (auto &it : buffer_hash){
        if (buffer_hash.right.count(it.right) < 2)
          files.erase(i);
        i++;
      }
      if (files.size() < 2)
        return EXIT_SUCCESS;
      pos += block_size;

    } while(check);

    size_t sz = files.begin()->right;
    for(auto &it : files){
      if (sz != it.right){
        std::cout << std::endl;
        sz = it.right;
      }
      std::cout << fs::system_complete(it.left) << std::endl;
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