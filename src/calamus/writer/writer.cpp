#include "amanuensis/calamus/writer.hpp"
#include "amanuensis/core/stable-hash.hpp"
#include "calamus/canonical.hpp"
#include "calamus/writer/body-writer.hpp"
#include "calamus/writer/changes.hpp"
#include "calamus/writer/header-writer.hpp"

#include <fstream>
#include <string>
#include <utility>
#include <variant>

namespace amanuensis::calamus {

static WriteResult Fail(std::string error)
{
  return WriteResult{false, std::string(), std::move(error)};
}

WriteResult Writer::WriteToString(
    const Header& header,
    const core::Value& entity,
    const core::Value& base,
    const WriterOptions& options
)
{
  const auto* entityObject = std::get_if<core::OrderedMap<core::Value>>(&entity.data);
  if (entityObject == nullptr) {
    return Fail("The entity must be an Object");
  }
  if (auto error = ValidateHeader(header)) {
    return Fail(std::move(*error));
  }

  core::Value canonicalBase = Canonicalise(base);
  Changes changes = FindChanges(canonicalBase, Canonicalise(entity), options.atomic);

  std::string body;
  BodyWriter bodyWriter(body, changes);
  if (!bodyWriter.WriteObject(*entityObject, &base, false, changes.whole.contains({}), 0)) {
    return Fail(bodyWriter.Error());
  }

  std::string text = WriteHeader(header, core::StableHash(canonicalBase));
  if (!body.empty()) {
    text.push_back('\n');
    text.append(body);
  }
  return WriteResult{true, std::move(text), std::string()};
}

WriteResult Writer::WriteToFile(
    const Header& header,
    const core::Value& entity,
    const core::Value& base,
    const std::filesystem::path& path,
    const WriterOptions& options
)
{
  WriteResult result = WriteToString(header, entity, base, options);
  if (!result.succeeded) {
    return result;
  }
  std::ofstream outputFile(path, std::ios::binary);
  if (!outputFile.is_open()) {
    return Fail("Could not open file: " + path.string());
  }
  outputFile.write(result.text.data(), static_cast<std::streamsize>(result.text.size()));
  if (!outputFile.good()) {
    return Fail("Could not write file: " + path.string());
  }
  return result;
}

} // namespace amanuensis::calamus
