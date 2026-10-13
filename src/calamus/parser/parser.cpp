#include "calamus/parser/parser.hpp"
#include "calamus/parser/body/body-line-parser.hpp"
#include "calamus/parser/body/tree-builder.hpp"
#include "calamus/parser/header/header-parser.hpp"
#include "calamus/parser/source-text.hpp"

#include <cstddef>
#include <utility>
#include <vector>

namespace amanuensis::calamus {

static ReadResult Fail(core::ParseError error)
{
  return ReadResult{false, Document{}, std::move(error)};
}

static bool IsHeaderLine(const SourceLine& line)
{
  return line.text.front() == '@';
}

ReadResult ParseDocument(std::string_view input)
{
  SourceText source;
  if (auto error = SplitLines(input, source)) {
    return Fail(*error);
  }

  HeaderParser headerParser;
  std::size_t index = 0;
  for (; index < source.lines.size() && IsHeaderLine(source.lines[index]); ++index) {
    if (auto error = headerParser.ParseLine(source.lines[index])) {
      return Fail(*error);
    }
  }
  if (auto error = headerParser.CheckRequired(source, index)) {
    return Fail(*error);
  }

  BodyLineParser bodyLineParser;
  std::vector<BodyLine> bodyLines;
  bodyLines.reserve(source.lines.size() - index);
  for (; index < source.lines.size(); ++index) {
    BodyLine bodyLine;
    if (auto error = bodyLineParser.Parse(source.lines[index], bodyLine)) {
      return Fail(*error);
    }
    bodyLines.push_back(std::move(bodyLine));
  }

  Document document;
  document.header = headerParser.TakeHeader();
  TreeBuilder builder(bodyLines, document.fields);
  if (auto error = builder.Build(document.body)) {
    return Fail(*error);
  }
  return ReadResult{true, std::move(document), {}};
}

} // namespace amanuensis::calamus
