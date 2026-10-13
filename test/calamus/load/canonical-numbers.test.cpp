#include <cimmerian/test.hpp>
#include "load-support.hpp"

#include <cmath>
#include <limits>
#include <string>
#include <variant>
#include <vector>

using namespace calamus_load_test;

DESCRIBE("Calamus load", {
  DESCRIBE("Canonical numbers", {
    IT("writes a double that is exactly a float as the shortest float text", {
      ASSERT_EQUAL(CanonicalText(static_cast<double>(0.1f)), std::string("0.1"));
      ASSERT_EQUAL(CanonicalText(16777216.0), std::string("16777216.0"));
      ASSERT_EQUAL(CanonicalText(-0.0), std::string("-0.0"));
      ASSERT_EQUAL(CanonicalText(1.0), std::string("1.0"));
    });

    IT("writes any other double as the shortest double text", {
      ASSERT_EQUAL(CanonicalText(0.1), std::string("0.1"));
      ASSERT_EQUAL(CanonicalText(1.0 / 3.0), std::string("0.3333333333333333"));
      ASSERT_EQUAL(CanonicalText(1e300), std::string("1e+300"));
      ASSERT_EQUAL(CanonicalText(5e-324), std::string("5e-324"));
    });

    IT("reads the canonical text back as the canonical double", {
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(static_cast<double>(0.1f))), Bits(0.1));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(0.1)), Bits(0.1));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(-0.0)), Bits(-0.0));
      ASSERT_EQUAL(Bits(calamus::CanonicalDouble(1.0 / 3.0)), Bits(1.0 / 3.0));
    });

    IT("is idempotent", {
      for (double value :
           {static_cast<double>(0.1f), static_cast<double>(46.103531f), 0.1, 1.0 / 3.0,
            3.4028234663852886e38, 1e300, 1e-45}) {
        double once = calamus::CanonicalDouble(value);
        ASSERT_EQUAL(Bits(calamus::CanonicalDouble(once)), Bits(once));
      }
    });

    IT("replaces doubles at any depth and leaves everything else alone", {
      core::Value value = Json(R"({"a":1,"b":"s","c":[0.0,{"d":0.0}],"e":true})");
      auto& object = std::get<core::OrderedMap<core::Value>>(value.data);
      auto& array = std::get<std::vector<core::Value>>(object.Get("c").data);
      array[0] = core::Value{static_cast<double>(0.1f)};
      std::get<core::OrderedMap<core::Value>>(array[1].data)
          .Insert("d", core::Value{static_cast<double>(0.2f)});
      object.Insert("f", core::Value{std::numeric_limits<double>::quiet_NaN()});

      core::Value canonical = calamus::Canonicalise(value);
      auto& canonicalObject = std::get<core::OrderedMap<core::Value>>(canonical.data);
      ASSERT_TRUE(std::isnan(std::get<double>(canonicalObject.Get("f").data)));
      // JSON can't write NaN, so swap it out before comparing the rest.
      canonicalObject.Insert("f", core::Value{});
      ASSERT_EQUAL(
          Minified(canonical),
          std::string(R"({"a":1,"b":"s","c":[0.1,{"d":0.2}],"e":true,"f":null})")
      );
    });
  });
});
