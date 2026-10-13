#include <cimmerian/test.hpp>
#include "atomic-support.hpp"
#include <amanuensis/serialization/serialization.hpp>

#include <string>
#include <variant>

using namespace calamus_atomic_test;

DESCRIBE("Atomic objects", {
  DESCRIBE("AtomicPaths", {
    IT("finds atomic objects through fields, Serialise members, optionals and maps", {
      Door door;
      door.highlight = Colour{};
      door.sockets["hinge"] = Quaternion{};
      door.sockets["handle"] = Quaternion{};
      door.keyframes.push_back(Quaternion{});
      ASSERT_EQUAL(
          Paths(amanuensis::AtomicPaths(door)),
          std::string("[orientation] [light.tint] [highlight] [sockets.handle] [sockets.hinge]")
      );
    });

    IT("skips an empty optional and never looks inside arrays", {
      ASSERT_EQUAL(
          Paths(amanuensis::AtomicPaths(Door{})), std::string("[orientation] [light.tint]")
      );
    });

    IT("gives the root path for an atomic value",
       { ASSERT_EQUAL(Paths(amanuensis::AtomicPaths(Quaternion{})), std::string("[]")); });

    IT("puts each path where ToValue puts the object", {
      Door door;
      door.highlight = Colour{};
      door.sockets["hinge"] = Quaternion{};
      core::Value value = amanuensis::ToValue(door);
      for (const core::KeyPath& path : amanuensis::AtomicPaths(door)) {
        const core::Value* current = &value;
        for (const std::string& key : path) {
          current = &std::get<core::OrderedMap<core::Value>>(current->data).Get(key);
        }
        ASSERT_TRUE(std::holds_alternative<core::OrderedMap<core::Value>>(current->data));
      }
    });
  });
});
