#include <iostream>
#include <boost/program_options.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iomanip>
#include <string>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, const char* argv[])
{
  po::options_description desc("options");
  po::variables_map vm;
  fs::path input_path;

  desc.add_options()
          ("help,h", "show help")
          ("input,i", po::value<std::vector<std::string>>(), "name input file")
          ("block-size,b",  po::value<size_t>()->default_value(5), "block size in bytes. default 5")
          ;
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    std::vector<std::string> input_files;
    if (vm.count("input"))
    {
      input_files = vm["input"].as<std::vector<std::string>>();
      for (auto s:input_files)
        std::cout << s << std::endl;
    }

  }
  catch (std::exception& ex)
  {
    std::cout << desc << std::endl;
  }

  return 0;
}