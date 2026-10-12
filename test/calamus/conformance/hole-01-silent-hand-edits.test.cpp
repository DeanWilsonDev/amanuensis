#include <cimmerian/test.hpp>
#include "conformance-support.hpp"

#include <cstddef>
#include <string>

using namespace conformance_test;

DESCRIBE("Calamus conformance", {
  DESCRIBE("Hole 1: silent hand edits", {
    IT("promotes a value changed without its '*', and healing adds the mark", {
      std::string edited = Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"gate\"");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_TRUE(loaded.needsRewrite);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"gate\""));
      ASSERT_TRUE(
          Heal(edited, Prefab).find("\n* name : \"gate\" | \"door\"\n") != std::string::npos
      );
    });

    IT("follows the new prefab when the prefab changed under the edit", {
      std::string edited = Replace(Write(Door, Prefab), "  name : \"door\"", "  name : \"gate\"");
      calamus::LoadResult loaded = Load(edited, EditedPrefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"wooden door\""));
    });

    IT("keeps an override whose '*' was deleted", {
      std::string edited = Replace(Write(Door, Prefab), "* locked", "  locked");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "locked")), std::string("true"));
      ASSERT_EQUAL(Heal(edited, Prefab), Write(Door, Prefab));
    });

    IT("resets a field when its recorded default is copied over the value", {
      std::string edited =
          Replace(Write(Door, Prefab), "* locked : true | false", "* locked : false | false");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_TRUE(loaded.needsRewrite);
      ASSERT_TRUE(Heal(edited, Prefab).find("\n  locked : false\n") != std::string::npos);
    });

    IT("resets a field when its line is deleted", {
      std::string edited = Replace(Write(Door, Prefab), "* locked : true | false\n", "");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "locked")), std::string("false"));
      ASSERT_TRUE(Heal(edited, Prefab).find("\n  locked : false\n") != std::string::npos);
    });

    IT("ignores a deleted recorded default", {
      std::string written = Write(Door, Prefab);
      std::string edited = Replace(written, "* locked : true | false", "* locked : true");
      calamus::LoadResult loaded = Load(edited, Prefab);
      ASSERT_FALSE(loaded.needsRewrite);
      ASSERT_EQUAL(Write(loaded.value, Prefab), written);
    });

    IT("lists overrides the prefab moved under, and the next save records the new default", {
      std::string written = Write(Door, Prefab);
      calamus::LoadResult loaded = Load(written, EditedPrefab);
      ASSERT_EQUAL(loaded.moved.size(), std::size_t{1});
      ASSERT_EQUAL(loaded.moved[0], core::KeyPath({"position", "x"}));
      ASSERT_TRUE(
          Write(loaded.value, EditedPrefab).find("*   x : 46.5 | 10.0\n") != std::string::npos
      );
    });

    IT("keeps the values of a hand-written file with no @base_hash", {
      std::string handWritten = "@version 1\n@type Door\n\n  name : \"gate\"\n  locked : false\n";
      calamus::LoadResult loaded = Load(handWritten, Prefab);
      ASSERT_EQUAL(Minified(Get(loaded.value, "name")), std::string("\"gate\""));
      ASSERT_TRUE(loaded.needsRewrite);
      std::string healed = Heal(handWritten, Prefab);
      ASSERT_TRUE(healed.find("@base_hash ") != std::string::npos);
      ASSERT_TRUE(healed.find("\n* name : \"gate\" | \"door\"\n") != std::string::npos);
    });
  });
});
