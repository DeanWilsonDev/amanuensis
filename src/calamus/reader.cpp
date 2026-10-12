#include "amanuensis/calamus/reader.hpp"
#include "calamus/parser.hpp"

#include <fstream>
#include <sstream>
#include <string>

namespace amanuensis::calamus {

ReadResult Reader::ParseString(std::string_view text)
{
  return ParseDocument(text);
}

ReadResult Reader::ParseFile(const std::filesystem::path& path)
{
  std::ifstream inputFile(path, std::ios::binary);
  if (!inputFile.is_open()) {
    return ReadResult{
        false, Document{}, core::ParseError{"Could not open file: " + path.string(), 0, 0}
    };
  }

  std::ostringstream contentStream;
  contentStream << inputFile.rdbuf();
  std::string fileContent = contentStream.str();

  return ParseDocument(fileContent);
}

} // namespace amanuensis::calamus
