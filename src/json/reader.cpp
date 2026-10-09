#include "amanuensis/json/reader.hpp"
#include "amanuensis/core/parse-result.hpp"
#include "parser.hpp"

#include <cerrno>
#include <cstdlib>
#include <string_view>
#include <sstream>
#include <fstream>

namespace amanuensis::json {

core::ParseResult Reader::ParseString(std::string_view text)
{
  return Parser(text).Parse();
}

core::ParseResult Reader::ParseFile(const std::filesystem::path& path)
{
  std::ifstream inputFile(path, std::ios::binary);
  if (!inputFile.is_open()) {
    return core::ParseResult{
        false, core::Value(), core::ParseError{"Could not open file: " + path.string(), 0, 0}
    };
  }

  std::ostringstream contentStream;
  contentStream << inputFile.rdbuf();
  std::string fileContent = contentStream.str();

  return Parser(fileContent).Parse();
}

} // namespace amanuensis::json
