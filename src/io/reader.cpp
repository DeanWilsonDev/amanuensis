#include "amanuensis/io/reader.hpp"
#include "amanuensis/io/parser-result.hpp"
#include "parser.hpp"

#include <cerrno>
#include <cstdlib>
#include <string_view>
#include <sstream>
#include <fstream>

namespace Amanuensis {

ParseResult Reader::ParseString(std::string_view text)
{
  return Parser(text).Parse();
}

ParseResult Reader::ParseFile(const std::filesystem::path& path)
{
  std::ifstream inputFile(path, std::ios::binary);
  if (!inputFile.is_open()) {
    return ParseResult{false, Value(), ParseError{"Could not open file: " + path.string(), 0, 0}};
  }

  std::ostringstream contentStream;
  contentStream << inputFile.rdbuf();
  std::string fileContent = contentStream.str();

  return Parser(fileContent).Parse();
}

} // namespace Amanuensis
