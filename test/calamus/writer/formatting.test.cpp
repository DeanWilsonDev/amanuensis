#include <cimmerian/test.hpp>
#include "writer-support.hpp"

#include <string>

using namespace calamus_writer_test;

DESCRIBE("Calamus writer", {
  DESCRIBE("Formatting", {
    IT("quotes keys that aren't identifiers", {
      core::Value entity = Json(R"({"door frame":1,"a.b":2,"_ok1":3,"":4,"9":5})");
      ASSERT_EQUAL(
          BodyOf(entity, entity),
          std::string("  \"door frame\" : 1\n  \"a.b\" : 2\n  _ok1 : 3\n  \"\" : 4\n  \"9\" : 5\n")
      );
    });

    IT("writes strings quoted, with JSON's escapes", {
      core::Value entity = Json(R"({"s":"a \"b\" | c # d\né"})");
      ASSERT_EQUAL(
          BodyOf(entity, entity), std::string("  s : \"a \\\"b\\\" | c # d\\n\xC3\xA9\"\n")
      );
    });

    IT("writes an empty object as a bare block line and an empty array as []", {
      core::Value entity = Json(R"({"light":{},"tags":[]})");
      ASSERT_EQUAL(BodyOf(entity, entity), std::string("  light:\n  tags : []\n"));
    });

    IT("writes arrays without objects inline, nesting them", {
      core::Value entity = Json(R"({"grid":[[1,2],[3],[]],"mixed":[null,true,"s",1.5]})");
      ASSERT_EQUAL(
          BodyOf(entity, entity),
          std::string("  grid : [[1, 2], [3], []]\n  mixed : [null, true, \"s\", 1.5]\n")
      );
    });

    IT("writes any array that holds an object as '-' items", {
      core::Value entity = Json(R"({"list":[1,{"x":2},[3,{"y":4}],[],{}]})");
      ASSERT_EQUAL(
          BodyOf(entity, entity), std::string(
                                      "  list:\n"
                                      "    - 1\n"
                                      "    -\n"
                                      "      x : 2\n"
                                      "    -\n"
                                      "      - 3\n"
                                      "      -\n"
                                      "        y : 4\n"
                                      "    - []\n"
                                      "    -\n"
                                  )
      );
    });
  });
});
