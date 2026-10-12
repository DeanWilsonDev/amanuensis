#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 11: small things", {
    IT("rejects duplicate keys", {
      ASSERT_FALSE(
          calamus::Reader::ParseString("@version 1\n@type Door\n  x : 1\n  x : 2\n").succeeded
      );
    });

    IT("reads CRLF and trailing whitespace the same as clean LF", {
      std::string written = Write(Door, Prefab);
      std::string messy;
      for (char character : written) {
        if (character == '\n') {
          messy += " \t\r\n";
        }
        else {
          messy += character;
        }
      }
      calamus::LoadResult loaded = Load(messy, Prefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Write(loaded.value, Prefab), written);
    });

    IT("writes LF only and exactly one final newline", {
      std::string written = Write(Door, Prefab);
      ASSERT_TRUE(written.find('\r') == std::string::npos);
      ASSERT_TRUE(written.ends_with("\n") && !written.ends_with("\n\n"));
    });
  });
});
