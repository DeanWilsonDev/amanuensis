#include "amanuensis/reader.hpp"
#include "amanuensis/parser.hpp"

#include <cerrno>
#include <charconv>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace Amanuensis {

Reader::Reader() {}

ParseResult Reader::ParseString(std::string_view text)
{
  parser.SetInput(text);
  auto result = parser.ParseValue();
  if (!result.succeeded) {
    return result;
  }

  // Ensure there's no trailing non-whitespace content
  parser.SkipWhitespace();
  if (!parser.IsAtEnd()) {
    return parser.MakeError(
        std::string("Unexpected content after JSON value: '") + parser.Peek() + "'"
    );
  }

  return result;
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

  return ParseString(fileContent);
}

} // namespace Amanuensis
