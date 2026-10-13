#include <cimmerian/test.hpp>
#include "atomic-support.hpp"
#include <amanuensis/serialization/serialization.hpp>

#include <filesystem>
#include <optional>
#include <vector>

using namespace calamus_atomic_test;

DESCRIBE("Atomic objects", {
  DESCRIBE("AMANUENSIS_ATOMIC", {
    IT("declares a type atomic and no other", {
      ASSERT_TRUE(amanuensis::IsAtomic<Quaternion>);
      ASSERT_TRUE(amanuensis::IsAtomic<Colour>);
      ASSERT_FALSE(amanuensis::IsAtomic<atomic_test::Vec3>);
      ASSERT_FALSE(amanuensis::IsAtomic<double>);
      ASSERT_FALSE(amanuensis::IsAtomic<std::optional<Quaternion>>);
      ASSERT_FALSE(amanuensis::IsAtomic<std::vector<Quaternion>>);
    });
  });

  AFTER_ALL({ std::filesystem::remove(TestFile()); });
});
