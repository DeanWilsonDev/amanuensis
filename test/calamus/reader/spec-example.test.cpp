#include <cimmerian/test.hpp>
#include "reader-support.hpp"

#include <cstdint>
#include <string>

using namespace calamus_reader_test;

DESCRIBE("Calamus reader", {
  DESCRIBE("The spec's example", {
    IT("reads the header", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      const calamus::Header& header = result.document.header;
      ASSERT_EQUAL(header.version, 3);
      ASSERT_EQUAL(header.type, std::string("Door"));
      REQUIRE_TRUE(header.id.has_value());
      ASSERT_EQUAL(*header.id, std::uint64_t{0x3f9c2a7be1d04c58});
      REQUIRE_TRUE(header.base.has_value());
      ASSERT_EQUAL(*header.base, std::string("prefabs/wooden_door"));
      REQUIRE_TRUE(header.baseHash.has_value());
      ASSERT_EQUAL(*header.baseHash, std::uint64_t{0x9f2c41e07b3d5a18});
    });

    IT("reads the body as written", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      ASSERT_EQUAL(
          Minified(result.document.body),
          std::string(
              R"({"position":{"x":46.103531,"y":108.07968,"z":0.0},)"
              R"("scale":{"x":1.0,"y":1.0,"z":1.0},)"
              R"("orientation":{"x":0.0,"y":0.0,"z":-0.069125,"w":0.997608},)"
              R"("name":"","flags":160,"locked":true,"tags":["puzzle","act1"],)"
              R"("waypoints":[{"x":0.0,"z":2.5},{"x":4.0,"z":2.5}],)"
              R"("light":{"intensity":2.5,"colour":{"r":1.0,"g":0.9,"b":0.8}}})"
          )
      );
    });

    IT("lists the fields in document order with their marks", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      std::string fields;
      for (const calamus::Field& field : result.document.fields) {
        fields += (fields.empty() ? "" : " ") + field.path.back() + (field.marked ? "*" : "");
      }
      ASSERT_EQUAL(
          fields, std::string(
                      "x* y* z x y z x* y* z* w* name flags locked* tags waypoints "
                      "intensity* r g b"
                  )
      );
    });

    IT("keeps each field's line and recorded default", {
      auto result = calamus::Reader::ParseString(SpecExample);
      REQUIRE_TRUE(result.succeeded);
      const calamus::Field& positionX = result.document.fields[0];
      ASSERT_EQUAL(positionX.line, 8);
      REQUIRE_TRUE(positionX.recordedDefault.has_value());
      ASSERT_EQUAL(Minified(*positionX.recordedDefault), std::string("0.0"));
      ASSERT_FALSE(result.document.fields[2].recordedDefault.has_value());
    });
  });
});
