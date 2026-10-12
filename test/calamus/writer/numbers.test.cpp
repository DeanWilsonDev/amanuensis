#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <limits>
#include <string>
#include <variant>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
  DESCRIBE("Numbers", {
    IT("writes a float field without float noise, and compares it canonically", {
      core::Value entity = Json(R"({"x":0.0})");
      std::get<core::OrderedMap<core::Value>>(entity.data)
          .Insert("x", core::Value{static_cast<double>(0.1f)});
      ASSERT_EQUAL(BodyOf(entity, Json(R"({"x":0.0})")), std::string("* x : 0.1 | 0.0\n"));
      ASSERT_EQUAL(BodyOf(entity, Json(R"({"x":0.1})")), std::string("  x : 0.1\n"));
    });

    IT("writes doubles with a '.0' or an exponent, and integers in plain decimal", {
      ASSERT_EQUAL(
          BodyOf(
              Json(
                  R"({"a":2.0,"b":1e300,"c":-0.0,"d":-9223372036854775808,"e":0.3333333333333333})"
              ),
              Json(
                  R"({"a":2.0,"b":1e300,"c":-0.0,"d":-9223372036854775808,"e":0.3333333333333333})"
              )
          ),
          std::string(
              "  a : 2.0\n  b : 1e+300\n  c : -0.0\n  d : -9223372036854775808\n"
              "  e : 0.3333333333333333\n"
          )
      );
    });

    IT("fails on NaN or infinity, naming the field", {
      core::Value entity = Json(R"({"light":{"intensity":0.0}})");
      std::get<core::OrderedMap<core::Value>>(
          std::get<core::OrderedMap<core::Value>>(entity.data).Get("light").data
      )
          .Insert("intensity", core::Value{std::numeric_limits<double>::infinity()});
      ASSERT_EQUAL(
          Write(entity, Json("{}")),
          std::string(
              "error: Can't write NaN or infinity at light.intensity; Calamus has no "
              "syntax for them"
          )
      );
    });

    IT("fails on NaN in a recorded default", {
      core::Value base = Json(R"({"x":0.0})");
      std::get<core::OrderedMap<core::Value>>(base.data).Insert(
          "x", core::Value{std::numeric_limits<double>::quiet_NaN()}
      );
      ASSERT_EQUAL(
          Write(Json(R"({"x":1.0})"), base),
          std::string(
              "error: Can't write NaN or infinity in the base at x; Calamus has no "
              "syntax for them"
          )
      );
    });
  });
});
