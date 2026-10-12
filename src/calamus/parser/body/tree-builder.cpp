#include "calamus/parser/body/tree-builder.hpp"

#include <utility>
#include <variant>

namespace amanuensis::calamus {

using Array = std::vector<core::Value>;
using Object = core::OrderedMap<core::Value>;

static bool IsWholeValueBlock(const core::Value& block)
{
  const auto* object = std::get_if<Object>(&block.data);
  return object == nullptr || object->Size() == 0;
}

MaybeError TreeBuilder::Build(core::Value& root)
{
  root = core::Value{Object{}};
  core::KeyPath path;
  return this->ReadChildren(0, root, path, false);
}

MaybeError
TreeBuilder::ReadChildren(int depth, core::Value& container, core::KeyPath& path, bool insideArray)
{
  while (this->index < this->lines.size() && this->lines[this->index].depth >= depth) {
    BodyLine& line = this->lines[this->index];
    if (auto error = this->CheckDepth(line, depth)) {
      return error;
    }
    if (auto error = this->CheckKind(line, container, depth)) {
      return error;
    }
    if (std::holds_alternative<std::monostate>(container.data)) {
      container = line.IsItem() ? core::Value{Array{}} : core::Value{Object{}};
    }

    ++this->index;
    auto error = line.IsItem() ? this->ReadItem(line, container, path, depth)
                               : this->ReadKeyed(line, container, path, depth, insideArray);
    if (error) {
      return error;
    }
  }
  return std::nullopt;
}

MaybeError TreeBuilder::CheckDepth(const BodyLine& line, int depth) const
{
  if (line.depth == depth) {
    return std::nullopt;
  }
  // A block line reads every deeper line, so this one follows a line that
  // can't have lines under it, or skips a level.
  const char* message = "Indented more than one level under the line above";
  if (this->index == 0) {
    message = "The first line of the body can't be indented";
  }
  else if (line.depth == depth + 1 && this->lines[this->index - 1].depth == depth) {
    message = "Only a 'key:' or '-' block line can have lines indented under it";
  }
  return core::ParseError{message, line.number, line.ContentColumn()};
}

MaybeError
TreeBuilder::CheckKind(const BodyLine& line, const core::Value& container, int depth) const
{
  if (std::holds_alternative<std::monostate>(container.data) ||
      line.IsItem() == std::holds_alternative<Array>(container.data)) {
    return std::nullopt;
  }
  const char* message = depth == 0 ? "'-' items can only appear inside an array block"
                                   : "A block can't mix fields and '-' items";
  return core::ParseError{message, line.number, line.ContentColumn()};
}

MaybeError
TreeBuilder::ReadItem(BodyLine& line, core::Value& container, core::KeyPath& path, int depth)
{
  core::Value item = std::move(line.value);
  if (line.kind == LineKind::ItemBlock) {
    if (auto error = this->ReadBlock(depth + 1, item, path, true)) {
      return error;
    }
  }
  std::get<Array>(container.data).push_back(std::move(item));
  return std::nullopt;
}

MaybeError TreeBuilder::ReadKeyed(
    BodyLine& line,
    core::Value& container,
    core::KeyPath& path,
    int depth,
    bool insideArray
)
{
  auto& object = std::get<Object>(container.data);
  if (object.Contains(line.key)) {
    return core::ParseError{"Duplicate key '" + line.key + "'", line.number, line.keyColumn};
  }

  path.push_back(line.key);
  core::Value value = std::move(line.value);
  bool isField = !insideArray;
  if (line.kind == LineKind::Block) {
    if (auto error = this->ReadBlock(depth + 1, value, path, insideArray)) {
      return error;
    }
    isField = isField && IsWholeValueBlock(value);
  }
  if (isField) {
    this->fields.push_back(Field{path, value, line.marked, line.recordedDefault, line.number});
  }
  path.pop_back();

  object.Insert(line.key, std::move(value));
  return std::nullopt;
}

MaybeError
TreeBuilder::ReadBlock(int depth, core::Value& block, core::KeyPath& path, bool insideArray)
{
  block = core::Value();
  if (auto error = this->ReadChildren(depth, block, path, insideArray)) {
    return error;
  }
  if (std::holds_alternative<std::monostate>(block.data)) {
    block = core::Value{Object{}};
  }
  return std::nullopt;
}

} // namespace amanuensis::calamus
