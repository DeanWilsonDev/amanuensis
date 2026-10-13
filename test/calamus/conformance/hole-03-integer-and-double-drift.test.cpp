#include <cimmerian/test.hpp>
#include "conformance-support.hpp"
#include <amanuensis/serialization/serialization.hpp>

#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 3: Integer and Double drift", {
    IT("keeps a whole Double a Double and an Integer an Integer", {
      core::Value entity = Json(R"({"d":1.0,"i":1,"big":1e21})");
      std::string written = Write(entity, Json("{}"));
      ASSERT_TRUE(written.find("* d : 1.0\n* i : 1\n* big : 1e+21\n") != std::string::npos);
      ASSERT_EQUAL(Minified(Load(written, Json("{}")).value), Minified(entity));
    });

    IT("loads a hand-typed Integer into a float field", {
      calamus::LoadResult loaded = Load(Replace(Write(Door, Prefab), "46.5", "46"), Prefab);
      ASSERT_EQUAL(amanuensis::FromValue<float>(Get(Get(loaded.value, "position"), "x")), 46.0f);
    });
  });
});
