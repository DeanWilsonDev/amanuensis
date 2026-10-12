#include <cimmerian/test.hpp>
#include "conformance-support.hpp"
#include <amanuensis/serialization/serialization.hpp>

#include <string>
#include <variant>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 4: float noise", {
    IT("writes a float's shortest text and reads it back as the same float", {
      for (float value : {0.1f, 46.103531f, 108.07968f, -0.069125f, 0.997608f, 1e-38f, 3.4e38f}) {
        core::Value entity{Object{}};
        std::get<Object>(entity.data).Insert("x", core::Value{static_cast<double>(value)});
        std::string written = Write(entity, Json(R"({"x":0.0})"));
        calamus::LoadResult loaded = Load(written, Json(R"({"x":0.0})"));
        ASSERT_EQUAL(amanuensis::FromValue<float>(Get(loaded.value, "x")), value);
        ASSERT_TRUE(written.find("0000") == std::string::npos);
      }
      ASSERT_TRUE(Write(Json(R"({"x":0.0})"), Json("{}")).find("* x : 0.0\n") != std::string::npos);
    });

    IT("reads every double back bit-identical", {
      for (double value : {0.1, 1.0 / 3.0, 5e-324, 1.7976931348623157e308, -0.0, 2.5e-310}) {
        core::Value entity{Object{}};
        std::get<Object>(entity.data).Insert("x", core::Value{value});
        calamus::LoadResult loaded = Load(Write(entity, Json("{}")), Json("{}"));
        ASSERT_EQUAL(Bits(std::get<double>(Get(loaded.value, "x").data)), Bits(value));
      }
    });

    IT("doesn't mark or promote a float field equal to its base", {
      core::Value base{Object{}};
      std::get<Object>(base.data).Insert("x", core::Value{static_cast<double>(0.1f)});
      std::string written = Write(base, base);
      ASSERT_TRUE(written.find("\n  x : 0.1\n") != std::string::npos);
      ASSERT_FALSE(Load(written, base).needsRewrite);
    });
  });
});
