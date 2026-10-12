#include <cimmerian/test.hpp>
#include <amanuensis/calamus/heal.hpp>
#include <amanuensis/calamus/writer.hpp>
#include <amanuensis/json/reader.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>

namespace calamus = amanuensis::calamus;
namespace core = amanuensis::core;

static core::Value Json(std::string_view text)
{
  return amanuensis::json::Reader::ParseString(text).value;
}

static std::filesystem::path TestFile()
{
  return std::filesystem::temp_directory_path() / "amanuensis_heal_test.calamus";
}

static void Save(const std::string& text)
{
  std::ofstream file(TestFile(), std::ios::binary | std::ios::trunc);
  file << text;
}

static std::string Saved()
{
  std::ifstream file(TestFile(), std::ios::binary);
  std::ostringstream content;
  content << file.rdbuf();
  return content.str();
}

static std::string Write(const core::Value& entity, const core::Value& base)
{
  calamus::Header header;
  header.version = 1;
  header.type = "Door";
  return calamus::Writer::WriteToString(header, entity, base).text;
}

static const core::Value Prefab = Json(R"({"name":"door","locked":false})");
static const core::Value Door = Json(R"({"name":"door","locked":true})");

DESCRIBE("Calamus heal", {
  IT("leaves a clean file alone", {
    std::string written = Write(Door, Prefab);
    Save(written);
    auto result = calamus::Heal(TestFile(), Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_FALSE(result.rewritten);
    ASSERT_EQUAL(Saved(), written);
  });

  IT("writes back a hand edit with its mark", {
    std::string edited = Write(Door, Prefab);
    edited.replace(edited.find("  name : \"door\""), 15, "  name : \"gate\"");
    Save(edited);

    auto result = calamus::Heal(TestFile(), Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_TRUE(result.rewritten);
    ASSERT_EQUAL(result.load.promoted.size(), std::size_t{1});
    ASSERT_EQUAL(Saved(), Write(Json(R"({"name":"gate","locked":true})"), Prefab));
  });

  IT("drops the mark from a field reset by hand", {
    std::string edited = Write(Door, Prefab);
    edited.replace(edited.find("* locked : true"), 15, "* locked : false");
    Save(edited);

    auto result = calamus::Heal(TestFile(), Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_TRUE(result.rewritten);
    ASSERT_EQUAL(Saved(), Write(Prefab, Prefab));
  });

  IT("doesn't rewrite a file whose base changed", {
    std::string written = Write(Door, Prefab);
    Save(written);
    core::Value newPrefab = Json(R"({"name":"wooden door","locked":false})");
    auto result = calamus::Heal(TestFile(), newPrefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_FALSE(result.rewritten);
    ASSERT_FALSE(result.load.baseHashMatches);
    ASSERT_EQUAL(Saved(), written);
  });

  IT("doesn't rewrite a clean file just for its line endings", {
    std::string crlf;
    for (char character : Write(Door, Prefab)) {
      crlf += character == '\n' ? std::string("\r\n") : std::string(1, character);
    }
    Save(crlf);
    auto result = calamus::Heal(TestFile(), Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_FALSE(result.rewritten);
    ASSERT_EQUAL(Saved(), crlf);
  });

  IT("reports a parse error with its line and column and leaves the file alone", {
    std::string broken = Write(Door, Prefab) + "\tx : 1\n";
    Save(broken);
    auto result = calamus::Heal(TestFile(), Prefab);
    ASSERT_FALSE(result.succeeded);
    ASSERT_EQUAL(result.error.message, std::string("Tabs aren't allowed in indentation"));
    ASSERT_EQUAL(result.error.column, 1);
    ASSERT_EQUAL(Saved(), broken);
  });

  IT("reports a missing file", {
    auto result = calamus::Heal(TestFile().string() + ".missing", Prefab);
    ASSERT_FALSE(result.succeeded);
    ASSERT_EQUAL(result.error.line, 0);
    ASSERT_TRUE(result.error.message.starts_with("Could not open file: "));
  });

  AFTER_ALL({ std::filesystem::remove(TestFile()); });
});
