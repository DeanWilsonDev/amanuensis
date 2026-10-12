#include "amanuensis/calamus/reader.hpp"
#include "amanuensis/core/diff.hpp"
#include "amanuensis/core/overlay.hpp"
#include "amanuensis/core/stable-hash.hpp"
#include "calamus/canonical.hpp"
#include "calamus/parser.hpp"
#include "calamus/paths.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

namespace amanuensis::calamus {

using Object = core::OrderedMap<core::Value>;

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

// -----------------------------------------------------------------------
// Load
// -----------------------------------------------------------------------

static bool Same(const core::Value& left, const core::Value& right)
{
  return core::Diff(left, right).empty();
}

LoadResult Reader::Load(const Document& document, const core::Value& base)
{
  core::Value canonicalBase = Canonicalise(base);

  LoadResult result;
  result.baseHashMatches =
      !document.header.baseHash || *document.header.baseHash == core::StableHash(canonicalBase);

  core::Value overrides{Object{}};
  for (const Field& field : document.fields) {
    const core::Value* baseValue = FindPath(canonicalBase, field.path);
    // Overlaying keeps the base's keys, so an empty object over an object is
    // no change.
    bool differs = baseValue == nullptr ||
                   !Same(*baseValue, core::Overlay(*baseValue, Canonicalise(field.value)));

    bool isOverride = false;
    if (field.marked) {
      if (differs) {
        isOverride = true;
        if (baseValue == nullptr) {
          result.additions.push_back(field.path);
        }
        if (field.recordedDefault &&
            (baseValue == nullptr || !Same(*baseValue, Canonicalise(*field.recordedDefault)))) {
          result.moved.push_back(field.path);
        }
      }
      else if (result.baseHashMatches) {
        result.reset.push_back(field.path);
      }
    }
    else if (differs) {
      if (result.baseHashMatches) {
        isOverride = true;
        result.promoted.push_back(field.path);
      }
      else if (baseValue != nullptr) {
        result.refreshed.push_back(field.path);
      }
      else {
        result.stale.push_back(field.path);
      }
    }

    if (isOverride) {
      SetPath(overrides, field.path, field.value);
      result.overrides.push_back(field.path);
    }
  }

  result.needsRewrite = !result.promoted.empty() || !result.reset.empty();
  result.value = core::Overlay(base, overrides);
  return result;
}

} // namespace amanuensis::calamus
