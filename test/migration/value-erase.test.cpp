#include <cimmerian/test.hpp>
#include "migration-support.hpp"
#include "support/values.hpp"

#include <cstddef>
#include <filesystem>
#include <string>

using namespace migration_test;

DESCRIBE("Migration", {
  DESCRIBE("Value::Erase", {
    IT("removes a key and keeps the rest in order", {
      core::Value object = Json(R"({"a":1,"b":2,"c":3,"d":4})");
      ASSERT_TRUE(object.Erase("b"));
      ASSERT_EQUAL(Minified(object), std::string(R"({"a":1,"c":3,"d":4})"));
      ASSERT_EQUAL(object.Get("d").AsInteger(), 4LL);
      object.Insert("b", core::Value{5LL});
      ASSERT_EQUAL(Minified(object), std::string(R"({"a":1,"c":3,"d":4,"b":5})"));
    });

    IT("says when the key wasn't there", {
      core::Value object = Json(R"({"a":1})");
      ASSERT_FALSE(object.Erase("missing"));
      ASSERT_EQUAL(object.Size(), std::size_t{1});
    });

    IT("throws on a value that isn't an Object", {
      core::Value array = core::Value::MakeArray();
      ASSERT_THROWS(array.Erase("a"), core::TypeMismatchError);
    });
  });

  AFTER_ALL({ std::filesystem::remove(TestFile()); });
});
