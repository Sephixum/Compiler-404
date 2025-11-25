#include "include/analyzers.hpp"
#include <print>
#include <fstream>
#include <filesystem>
#include <ranges>

namespace fs = std::filesystem;
namespace stdr = std::ranges;
namespace stdv = std::views;

auto main(int argc, char** argv) -> int 
{
  if (argc <= 1)
  {
    std::println("  [INFO] Usage...");
    std::println("    {} {}", argv[0], "code.txt");
    return EXIT_FAILURE;
  }
  else if(argc > 2)
  {
    std::println("[Help] Usage...");
    std::println("{} {}", argv[0], "code.txt");
    return EXIT_FAILURE;
  }

  auto input_file  = std::ifstream(fs::path{argv[1]});
  auto output_file = std::ofstream(fs::current_path()/"out.txt");

  if(not output_file.is_open())
  {
    std::println("[Error] output_file cannot be oppended !!");
    return EXIT_FAILURE;
  }

  if(not input_file.is_open())
  {
    std::println("[Error] input_file cannot be oppended !!");
    return EXIT_FAILURE;
  }

  auto line = std::string{};
  line.reserve(256);

  auto line_counter = std::size_t{0};
  while (std::getline(input_file, line))
  {
    ++line_counter;

    auto parse_bound = [&](std::string const& s)
    { 
      return analyzer::parse_all(s, line_counter);
    };

    auto lexeme_pipeline = line 
                         | stdv::split(' ')
                         | stdv::filter([](auto&& r)   { return not std::ranges::empty(r); })
                         | stdv::transform([](auto&& r){ return std::string{std::ranges::begin(r), std::ranges::end(r)};})
                         | stdv::transform(parse_bound)
                         | stdv::join_with(' ')
                         | stdr::to<std::string>();

    std::println(output_file, "{}", lexeme_pipeline);
  }

  return EXIT_SUCCESS;
}
