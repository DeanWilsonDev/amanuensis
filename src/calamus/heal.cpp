#include "amanuensis/calamus/heal.hpp"
#include "amanuensis/calamus/writer.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>

namespace amanuensis::calamus {

static HealResult Fail(core::ParseError error, LoadResult load = {})
{
  return HealResult{false, false, std::nullopt, std::move(load), std::move(error)};
}

HealResult Heal(
    const std::filesystem::path& path,
    const core::Value& base,
    const WriterOptions& options,
    const std::optional<Migration>& migration
)
{
  std::ifstream inputFile(path, std::ios::binary);
  if (!inputFile.is_open()) {
    return Fail(core::ParseError{"Could not open file: " + path.string(), 0, 0});
  }
  std::ostringstream contentStream;
  contentStream << inputFile.rdbuf();
  std::string fileContent = contentStream.str();
  inputFile.close();

  ReadResult read = Reader::ParseString(fileContent);
  if (!read.succeeded) {
    return Fail(read.error);
  }
  std::optional<int> migratedFrom;
  if (migration) {
    int fileVersion = read.document.header.version;
    core::MigrateResult migrated = Migrate(read.document, *migration);
    if (!migrated.succeeded) {
      return Fail(core::ParseError{migrated.error, 0, 0});
    }
    if (read.document.header.version != fileVersion) {
      migratedFrom = fileVersion;
    }
  }

  LoadResult load = Reader::Load(read.document, base);
  if (migratedFrom || !load.needsRewrite) {
    return HealResult{true, false, migratedFrom, std::move(load), {}};
  }

  WriteResult written = Writer::WriteToString(read.document.header, load.value, base, options);
  if (!written.succeeded) {
    return Fail(core::ParseError{written.error, 0, 0}, std::move(load));
  }
  if (written.text == fileContent) {
    return HealResult{true, false, std::nullopt, std::move(load), {}};
  }

  std::ofstream outputFile(path, std::ios::binary | std::ios::trunc);
  if (!outputFile.is_open()) {
    return Fail(
        core::ParseError{"Could not open file for writing: " + path.string(), 0, 0}, std::move(load)
    );
  }
  outputFile.write(written.text.data(), static_cast<std::streamsize>(written.text.size()));
  if (!outputFile.good()) {
    return Fail(core::ParseError{"Could not write file: " + path.string(), 0, 0}, std::move(load));
  }
  return HealResult{true, true, std::nullopt, std::move(load), {}};
}

} // namespace amanuensis::calamus
