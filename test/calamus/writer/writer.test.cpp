#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <string>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
  IT("writes an entity over its prefab", {
    calamus::Header header;
    header.version = 3;
    header.type = "Door";
    header.id = 0x3f9c2a7be1d04c58;
    header.base = "prefabs/wooden_door";
    auto result = calamus::Writer::WriteToString(header, Door, Prefab);
    REQUIRE_TRUE(result.succeeded);
    ASSERT_EQUAL(
        result.text, "@version 3\n"
                     "@type Door\n"
                     "@id 3f9c2a7be1d04c58\n"
                     "@base \"prefabs/wooden_door\"\n"
                     "@base_hash " +
                         Hash(Prefab) +
                         "\n"
                         "\n"
                         "* position:\n"
                         "*   x : 46.5 | 0.0\n"
                         "*   y : 108.25 | 0.0\n"
                         "    z : 0.0\n"
                         "  scale:\n"
                         "    x : 1.0\n"
                         "    y : 1.0\n"
                         "    z : 1.0\n"
                         "  name : \"\"\n"
                         "  flags : 160\n"
                         "* locked : true | false\n"
                         "  tags : [\"puzzle\", \"act1\"]\n"
                         "* waypoints:\n"
                         "*   -\n"
                         "*     x : 0.0\n"
                         "*     z : 2.5\n"
                         "*   -\n"
                         "*     x : 4.0\n"
                         "*     z : 2.5\n"
                         "* light:\n"
                         "*   intensity : 2.5 | 1.0\n"
                         "    colour:\n"
                         "      r : 1.0\n"
                         "      g : 0.9\n"
                         "      b : 0.8\n"
    );
  });

  IT("gives the same text every time", { ASSERT_EQUAL(Write(Door, Prefab), Write(Door, Prefab)); });
});
