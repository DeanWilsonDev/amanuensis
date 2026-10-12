#include "calamus/parser.hpp"

#include "calamus/syntax.hpp"
#include "core/cursor.hpp"
#include "core/number-format.hpp"
#include "core/quoted-string.hpp"

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace amanuensis::calamus {

namespace {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

using MaybeError = std::optional<core::ParseError>;

// One non-blank line of the input, without its line ending or trailing
// whitespace. Each line is read with its own Cursor, so a Cursor's columns
// are the line's columns and its line is always 1; errors are given the real
// line number afterwards.
struct SourceLine {
  std::string_view text;
  int number;
};

struct SourceText {
  std::vector<SourceLine> lines;
  int endLine = 1;
  int endColumn = 1;
};

MaybeError SplitLines(std::string_view input, SourceText& source)
{
  constexpr std::string_view byteOrderMark = "\xEF\xBB\xBF";
  if (input.starts_with(byteOrderMark)) {
    input.remove_prefix(byteOrderMark.size());
  }

  int number = 1;
  while (true) {
    std::size_t lineEnd = input.find('\n');
    std::string_view text = input.substr(0, lineEnd);
    if (lineEnd != std::string_view::npos && text.ends_with('\r')) {
      text.remove_suffix(1);
    }

    std::size_t carriageReturn = text.find('\r');
    if (carriageReturn != std::string_view::npos) {
      return core::ParseError{
          "A carriage return must be followed by a line feed", number,
          static_cast<int>(carriageReturn) + 1
      };
    }

    source.endLine = number;
    source.endColumn = static_cast<int>(text.size()) + 1;

    while (!text.empty() && (text.back() == ' ' || text.back() == '\t')) {
      text.remove_suffix(1);
    }
    if (!text.empty()) {
      source.lines.push_back(SourceLine{text, number});
    }

    if (lineEnd == std::string_view::npos) {
      return std::nullopt;
    }
    input.remove_prefix(lineEnd + 1);
    ++number;
  }
}

bool IsHexDigit(char character)
{
  return (character >= '0' && character <= '9') || (character >= 'a' && character <= 'f') ||
         (character >= 'A' && character <= 'F');
}

void SkipSpaces(core::Cursor& cursor)
{
  while (cursor.Peek() == ' ') {
    cursor.Advance();
  }
}

std::string_view ReadIdentifier(core::Cursor& cursor)
{
  std::size_t start = cursor.Position();
  while (IsIdentifierCharacter(cursor.Peek())) {
    cursor.Advance();
  }
  return cursor.Slice(start);
}

core::ParseError ErrorAt(std::string message, const SourceLine& line, int column)
{
  return core::ParseError{std::move(message), line.number, column};
}

core::ParseError ErrorHere(std::string message, const SourceLine& line, const core::Cursor& cursor)
{
  return ErrorAt(std::move(message), line, cursor.Column());
}

core::ParseError OnLine(core::ParseError error, const SourceLine& line)
{
  error.line = line.number;
  return error;
}

const char* CommentError = "Comments aren't supported in Calamus v1";

core::ParseResult ParseValue(core::Cursor& cursor);

core::ParseResult ParseInlineArray(core::Cursor& cursor)
{
  cursor.Advance(); // consume '['
  SkipSpaces(cursor);

  core::Value arrayValue{Array{}};
  auto& elements = std::get<Array>(arrayValue.data);

  if (cursor.Peek() == ']') {
    cursor.Advance();
    return core::ParseResult{true, std::move(arrayValue), {}};
  }

  while (true) {
    auto elementResult = ParseValue(cursor);
    if (!elementResult.succeeded) {
      return elementResult;
    }
    elements.push_back(std::move(elementResult.value));

    SkipSpaces(cursor);

    if (cursor.IsAtEnd()) {
      return cursor.MakeError("Unterminated array; an inline array must end on its own line");
    }
    if (cursor.Peek() == ']') {
      cursor.Advance();
      return core::ParseResult{true, std::move(arrayValue), {}};
    }
    if (cursor.Peek() != ',') {
      return cursor.MakeError(
          std::string("Expected ',' or ']' in array, got '") + cursor.Peek() + "'"
      );
    }
    cursor.Advance(); // consume ','

    SkipSpaces(cursor);
    if (cursor.Peek() == ']') {
      return cursor.MakeError("Trailing comma in array");
    }
  }
}

// null, true and false. Any other bare word is an unquoted string.
core::ParseResult ParseWord(core::Cursor& cursor)
{
  int line = cursor.Line();
  int column = cursor.Column();
  std::string_view word = ReadIdentifier(cursor);
  if (word == "null") {
    return core::ParseResult{true, core::Value(), {}};
  }
  if (word == "true") {
    return core::ParseResult{true, core::Value(true), {}};
  }
  if (word == "false") {
    return core::ParseResult{true, core::Value(false), {}};
  }
  return core::ParseResult{
      false, core::Value(),
      core::ParseError{"Strings must be quoted, as in \"" + std::string(word) + "\"", line, column}
  };
}

core::ParseResult ParseValue(core::Cursor& cursor)
{
  if (cursor.IsAtEnd()) {
    return cursor.MakeError("Expected a value");
  }

  char current = cursor.Peek();
  if (current == '"') {
    return core::ParseQuotedString(cursor);
  }
  if (current == '[') {
    return ParseInlineArray(cursor);
  }
  if (current == '-' || (current >= '0' && current <= '9')) {
    return core::ParseNumber(cursor);
  }
  if (IsIdentifierStart(current)) {
    return ParseWord(cursor);
  }
  if (current == '{') {
    return cursor.MakeError("Objects can't be written inline; open a block with 'key:' instead");
  }
  if (current == '#') {
    return cursor.MakeError(CommentError);
  }
  return cursor.MakeError(std::string("Unexpected character '") + current + "'");
}

struct HeaderState {
  Header header;
  bool hasVersion = false;
  bool hasType = false;
  bool hasId = false;
  bool hasBase = false;
  bool hasBaseHash = false;
};

std::optional<std::uint64_t> ParseHex(std::string_view text, std::size_t maxDigits)
{
  if (text.empty() || text.size() > maxDigits) {
    return std::nullopt;
  }
  for (char character : text) {
    if (!IsHexDigit(character)) {
      return std::nullopt;
    }
  }
  std::uint64_t value = 0;
  std::from_chars(text.data(), text.data() + text.size(), value, 16);
  return value;
}

MaybeError ParseHeaderLine(const SourceLine& line, HeaderState& state)
{
  core::Cursor cursor(line.text);
  cursor.Advance();

  std::string name(ReadIdentifier(cursor));
  if (name.empty()) {
    return ErrorHere("Expected a header name after '@'", line, cursor);
  }

  bool* seen = nullptr;
  if (name == "version") {
    seen = &state.hasVersion;
  }
  else if (name == "type") {
    seen = &state.hasType;
  }
  else if (name == "id") {
    seen = &state.hasId;
  }
  else if (name == "base") {
    seen = &state.hasBase;
  }
  else if (name == "base_hash") {
    seen = &state.hasBaseHash;
  }
  else {
    return ErrorAt("Unknown header line '@" + name + "'", line, 1);
  }
  if (*seen) {
    return ErrorAt("Repeated header line '@" + name + "'", line, 1);
  }
  *seen = true;

  if (cursor.IsAtEnd()) {
    return ErrorHere("Expected a value after '@" + name + "'", line, cursor);
  }
  if (cursor.Peek() != ' ') {
    return ErrorHere("Expected a space after '@" + name + "'", line, cursor);
  }
  SkipSpaces(cursor);

  int valueColumn = cursor.Column();
  std::string_view value = line.text.substr(cursor.Position());
  Header& header = state.header;

  if (name == "version") {
    int version = 0;
    auto [endPointer, errorCode] =
        std::from_chars(value.data(), value.data() + value.size(), version);
    if (value.front() < '1' || value.front() > '9' || errorCode != std::errc() ||
        endPointer != value.data() + value.size()) {
      return ErrorAt("@version must be a whole number of at least 1", line, valueColumn);
    }
    header.version = version;
  }
  else if (name == "type") {
    if (!IsTypeName(value)) {
      return ErrorAt(
          "@type must be a name: a letter or '_', then letters, digits, '_', '.' or ':'", line,
          valueColumn
      );
    }
    header.type = std::string(value);
  }
  else if (name == "id") {
    header.id = ParseHex(value, 16);
    if (!header.id) {
      return ErrorAt("@id must be 1 to 16 hex digits", line, valueColumn);
    }
  }
  else if (name == "base") {
    if (cursor.Peek() != '"') {
      return ErrorAt("@base must be a quoted string", line, valueColumn);
    }
    auto baseResult = core::ParseQuotedString(cursor);
    if (!baseResult.succeeded) {
      return OnLine(baseResult.error, line);
    }
    SkipSpaces(cursor);
    if (!cursor.IsAtEnd()) {
      return ErrorHere("Unexpected text after the @base string", line, cursor);
    }
    header.base = std::get<std::string>(baseResult.value.data);
  }
  else {
    header.baseHash = value.size() == 16 ? ParseHex(value, 16) : std::nullopt;
    if (!header.baseHash) {
      return ErrorAt("@base_hash must be exactly 16 hex digits", line, valueColumn);
    }
  }
  return std::nullopt;
}

enum class LineKind {
  Field,     // key : value
  Block,     // key:
  Item,      // - value
  ItemBlock, // -
};

struct BodyLine {
  int number = 0;
  bool marked = false;
  int depth = 0;
  LineKind kind = LineKind::Field;
  std::string key;
  int keyColumn = 0;
  core::Value value;
  std::optional<core::Value> recordedDefault;

  bool IsItem() const { return kind == LineKind::Item || kind == LineKind::ItemBlock; }
  int ContentColumn() const { return 3 + depth * 2; }
};

// Whatever may follow a value: an optional recorded default, then the end of
// the line.
MaybeError
ParseLineEnd(core::Cursor& cursor, const SourceLine& line, BodyLine& bodyLine, const char* after)
{
  SkipSpaces(cursor);
  if (cursor.IsAtEnd()) {
    return std::nullopt;
  }
  if (cursor.Peek() == '|' && bodyLine.kind == LineKind::Field && !bodyLine.recordedDefault) {
    cursor.Advance();
    SkipSpaces(cursor);
    if (cursor.IsAtEnd()) {
      return ErrorHere("Expected a recorded default after '|'", line, cursor);
    }
    auto defaultResult = ParseValue(cursor);
    if (!defaultResult.succeeded) {
      return OnLine(defaultResult.error, line);
    }
    bodyLine.recordedDefault = std::move(defaultResult.value);
    return ParseLineEnd(cursor, line, bodyLine, "the recorded default");
  }
  if (cursor.Peek() == '|' && bodyLine.kind == LineKind::Item) {
    return ErrorHere("A recorded default can only follow a field's value", line, cursor);
  }
  if (cursor.Peek() == '#') {
    return ErrorHere(CommentError, line, cursor);
  }
  return ErrorHere(std::string("Unexpected '") + cursor.Peek() + "' after " + after, line, cursor);
}

MaybeError ParseBodyLine(const SourceLine& line, BodyLine& bodyLine)
{
  core::Cursor cursor(line.text);
  bodyLine.number = line.number;

  char marker = cursor.Peek();
  if (marker == '@') {
    return ErrorAt("Header lines must come before the body", line, 1);
  }
  if (marker == '\t') {
    return ErrorAt("Tabs aren't allowed in indentation", line, 1);
  }
  if (marker != '*' && marker != ' ') {
    return ErrorAt("Expected '*' or a space in the marker column", line, 1);
  }
  bodyLine.marked = marker == '*';
  cursor.Advance();

  if (cursor.IsAtEnd()) {
    return ErrorAt("Expected a field after the marker", line, 2);
  }
  if (cursor.Peek() == '\t') {
    return ErrorAt("Tabs aren't allowed in indentation", line, 2);
  }
  if (cursor.Peek() != ' ') {
    return ErrorAt("Expected a space after the marker column", line, 2);
  }
  cursor.Advance();

  int spaces = 0;
  while (cursor.Peek() == ' ' || cursor.Peek() == '\t') {
    if (cursor.Peek() == '\t') {
      return ErrorHere("Tabs aren't allowed in indentation", line, cursor);
    }
    ++spaces;
    cursor.Advance();
  }
  if (cursor.IsAtEnd()) {
    return ErrorHere("Expected a field after the marker", line, cursor);
  }
  if (spaces % 2 != 0) {
    return ErrorHere("Indentation must be a multiple of two spaces", line, cursor);
  }
  bodyLine.depth = spaces / 2;

  if (cursor.Peek() == '-') {
    cursor.Advance();
    if (cursor.IsAtEnd()) {
      bodyLine.kind = LineKind::ItemBlock;
      return std::nullopt;
    }
    if (cursor.Peek() != ' ') {
      return ErrorHere("Expected a space after '-'", line, cursor);
    }
    SkipSpaces(cursor);
    bodyLine.kind = LineKind::Item;
    auto itemResult = ParseValue(cursor);
    if (!itemResult.succeeded) {
      return OnLine(itemResult.error, line);
    }
    bodyLine.value = std::move(itemResult.value);
    return ParseLineEnd(cursor, line, bodyLine, "the value");
  }

  bodyLine.keyColumn = cursor.Column();
  if (cursor.Peek() == '"') {
    auto keyResult = core::ParseQuotedString(cursor);
    if (!keyResult.succeeded) {
      return OnLine(keyResult.error, line);
    }
    bodyLine.key = std::get<std::string>(keyResult.value.data);
  }
  else if (IsIdentifierStart(cursor.Peek())) {
    bodyLine.key = std::string(ReadIdentifier(cursor));
  }
  else if (cursor.Peek() == '#') {
    return ErrorHere(CommentError, line, cursor);
  }
  else {
    return ErrorHere("Expected a key or a '-' item", line, cursor);
  }

  SkipSpaces(cursor);
  if (cursor.Peek() != ':') {
    return ErrorHere("Expected ':' after the key", line, cursor);
  }
  cursor.Advance();
  SkipSpaces(cursor);

  if (cursor.IsAtEnd()) {
    bodyLine.kind = LineKind::Block;
    return std::nullopt;
  }

  bodyLine.kind = LineKind::Field;
  auto valueResult = ParseValue(cursor);
  if (!valueResult.succeeded) {
    return OnLine(valueResult.error, line);
  }
  bodyLine.value = std::move(valueResult.value);
  return ParseLineEnd(cursor, line, bodyLine, "the value");
}

class TreeBuilder {
public:
  TreeBuilder(std::vector<BodyLine>& lines, std::vector<Field>& fields)
      : lines_(lines)
      , fields_(fields)
  {
  }

  MaybeError Build(core::Value& root)
  {
    root = core::Value{Object{}};
    core::KeyPath path;
    return ReadChildren(0, root, path, false);
  }

private:
  // A Null container takes its kind from its first line: fields make an Object,
  // '-' items an Array.
  MaybeError ReadChildren(int depth, core::Value& container, core::KeyPath& path, bool insideArray)
  {
    while (index_ < lines_.size() && lines_[index_].depth >= depth) {
      BodyLine& line = lines_[index_];

      if (line.depth > depth) {
        // A block line reads every deeper line, so this one follows a line that
        // can't have lines under it, or skips a level.
        const char* message = "Indented more than one level under the line above";
        if (index_ == 0) {
          message = "The first line of the body can't be indented";
        }
        else if (line.depth == depth + 1 && lines_[index_ - 1].depth == depth) {
          message = "Only a 'key:' or '-' block line can have lines indented under it";
        }
        return core::ParseError{message, line.number, line.ContentColumn()};
      }

      if (std::holds_alternative<std::monostate>(container.data)) {
        container = line.IsItem() ? core::Value{Array{}} : core::Value{Object{}};
      }
      else if (line.IsItem() != std::holds_alternative<Array>(container.data)) {
        const char* message = depth == 0 ? "'-' items can only appear inside an array block"
                                         : "A block can't mix fields and '-' items";
        return core::ParseError{message, line.number, line.ContentColumn()};
      }

      ++index_;
      if (line.IsItem()) {
        core::Value item = std::move(line.value);
        if (line.kind == LineKind::ItemBlock) {
          if (auto error = ReadBlock(depth + 1, item, path, true)) {
            return error;
          }
        }
        std::get<Array>(container.data).push_back(std::move(item));
        continue;
      }

      auto& object = std::get<Object>(container.data);
      if (object.Contains(line.key)) {
        return core::ParseError{"Duplicate key '" + line.key + "'", line.number, line.keyColumn};
      }

      path.push_back(line.key);
      core::Value value = std::move(line.value);
      bool isField = !insideArray;
      if (line.kind == LineKind::Block) {
        if (auto error = ReadBlock(depth + 1, value, path, insideArray)) {
          return error;
        }
        isField = isField && IsWholeValueBlock(value);
      }
      if (isField) {
        fields_.push_back(Field{path, value, line.marked, line.recordedDefault, line.number});
      }
      path.pop_back();

      object.Insert(line.key, std::move(value));
    }
    return std::nullopt;
  }

  // A block with no lines under it is an empty Object.
  MaybeError ReadBlock(int depth, core::Value& block, core::KeyPath& path, bool insideArray)
  {
    block = core::Value();
    if (auto error = ReadChildren(depth, block, path, insideArray)) {
      return error;
    }
    if (std::holds_alternative<std::monostate>(block.data)) {
      block = core::Value{Object{}};
    }
    return std::nullopt;
  }

  static bool IsWholeValueBlock(const core::Value& block)
  {
    const auto* object = std::get_if<Object>(&block.data);
    return object == nullptr || object->Size() == 0;
  }

  std::vector<BodyLine>& lines_;
  std::vector<Field>& fields_;
  std::size_t index_ = 0;
};

core::ParseError MissingHeader(const char* name, const SourceText& source, std::size_t bodyStart)
{
  std::string message = std::string("Missing the ") + name + " header line";
  if (bodyStart < source.lines.size()) {
    return core::ParseError{message, source.lines[bodyStart].number, 1};
  }
  return core::ParseError{message, source.endLine, source.endColumn};
}

ReadResult Fail(core::ParseError error)
{
  return ReadResult{false, Document{}, std::move(error)};
}

} // namespace

ReadResult ParseDocument(std::string_view input)
{
  SourceText source;
  if (auto error = SplitLines(input, source)) {
    return Fail(*error);
  }

  HeaderState headerState;
  std::size_t index = 0;
  for (; index < source.lines.size() && source.lines[index].text.front() == '@'; ++index) {
    if (auto error = ParseHeaderLine(source.lines[index], headerState)) {
      return Fail(*error);
    }
  }
  if (!headerState.hasVersion) {
    return Fail(MissingHeader("@version", source, index));
  }
  if (!headerState.hasType) {
    return Fail(MissingHeader("@type", source, index));
  }

  std::vector<BodyLine> bodyLines;
  bodyLines.reserve(source.lines.size() - index);
  for (; index < source.lines.size(); ++index) {
    BodyLine bodyLine;
    if (auto error = ParseBodyLine(source.lines[index], bodyLine)) {
      return Fail(*error);
    }
    bodyLines.push_back(std::move(bodyLine));
  }

  Document document;
  document.header = std::move(headerState.header);
  TreeBuilder builder(bodyLines, document.fields);
  if (auto error = builder.Build(document.body)) {
    return Fail(*error);
  }
  return ReadResult{true, std::move(document), {}};
}

} // namespace amanuensis::calamus
