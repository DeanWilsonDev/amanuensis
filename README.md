![Amanuensis logo](assets/amanuensis-logo.jpg)

# Amanuensis

A zero-dependency C++26 JSON read/write library. Minimal feature surface, insertion-order preserving, consumed via CMake `add_subdirectory`.

---

## Overview

Amanuensis reads and writes JSON conforming to [RFC 8259](https://www.rfc-editor.org/rfc/rfc8259). 
The library is deliberately minimal. It covers the subset of JSON that real first-party projects actually use, and nothing more.

---

## Goals

- Read and write JSON conforming to RFC 8259
- Preserve insertion order on write so generated files are stable under diffing
- Zero external dependencies — C++26 stdlib only
- Consumed via CMake `add_subdirectory`
- Human-readable output by default (pretty-printed, 2-space indent)
- Minified output mode for wire formats and size-sensitive use cases
- Clear error reporting on parse failure — line, column, and reason
- Low-boilerplate serialisation of user types via a tiered opt-in system

## Non-Goals

- JSON Schema validation
- Streaming / incremental parsing
- Binary formats (BSON, CBOR, MessagePack)
- Runtime-reflection-based automatic serialisation
- JSON5, JSONC, or other relaxed dialects
- Performance parity with SIMD-optimised parsers like simdjson

---

## Integration

Add Amanuensis as a subdirectory under `libs/` and link against it:

```cmake
add_subdirectory(libs/amanuensis)
target_link_libraries(your_target PRIVATE amanuensis)
```

Then include the umbrella header:

```cpp
#include <amanuensis.hpp>
```

Or include individual headers as needed:

```cpp
#include <amanuensis/core/value.hpp>                   // amanuensis::core::Value and its operations
#include <amanuensis/json/reader.hpp>                  // amanuensis::json::Reader
#include <amanuensis/json/writer.hpp>                  // amanuensis::json::Writer
#include <amanuensis/calamus/reader.hpp>               // amanuensis::calamus::Reader
#include <amanuensis/calamus/writer.hpp>               // amanuensis::calamus::Writer
#include <amanuensis/calamus/heal.hpp>                 // amanuensis::calamus::Heal
#include <amanuensis/serialization/serialization.hpp>  // ToValue, FromValue, AMANUENSIS_SERIALISABLE, AMANUENSIS_VERSION
```

---

## Building

### Library only

```bash
cmake -B build -DAMANUENSIS_BUILD_TESTS=OFF
cmake --build build
```

### With tests

Tests use [Cimmerian](https://github.com/DeanWilsonDev/Cimmerian), which is a git submodule at `external/cimmerian`. Tests are enabled by default when Amanuensis is the top-level project.

```bash
git submodule update --init
cmake -B build
cmake --build build
./build/test_amanuensis
```

---

## API

The data model and everything the formats share lives in `amanuensis::core`, the JSON reader and writer live in `amanuensis::json`, and the Calamus reader and writer live in `amanuensis::calamus`. The serialisation layer (`ToValue`, `FromValue`, `SerialTraits`) is in `amanuensis`. The operations on a `Value` are its member functions.

Until consumers have moved over, `<amanuensis/compat.hpp>` (pulled in by every public header) keeps the old names working:

- the `Amanuensis::` namespace spelling;
- `JsonValue`, `JsonValueType`, `JsonParseResult` and `JsonParseError`, as aliases of `core::Value`, `core::ValueType`, `core::ParseResult` and `core::ParseError`;
- `amanuensis::Json`, whose static functions forward to `Value`'s member functions: `Json::Get(value, key)` is `value.Get(key)`;
- `ToJson`, `FromJson`, `TryFromJson` and `FromJsonResult`, which forward to `ToValue`, `FromValue`, `TryFromValue` and `FromValueResult`, and `JsonTraits<T>` specialisations with `ToJson`/`FromJson` members, which are still picked up when there is no `SerialTraits<T>` one;
- `amanuensis::Reader`, `Writer` and `WriterOptions`, now in `amanuensis::json`, and `amanuensis::OrderedMap`, `ObjectIterator`, `Converter`, `ValueTraits` and the error types, now in `amanuensis::core`;
- the old header paths, such as `<amanuensis/io/reader.hpp>` and `<amanuensis/json-value.hpp>`, which forward to the new ones.

Define `AMANUENSIS_NO_COMPAT` to turn all of this off.

### Reading

`Reader` has two static methods and returns a result struct — it never throws on parse failure.

```cpp
auto result = amanuensis::json::Reader::ParseString(R"({"x": 1})");

if (!result.succeeded) {
    std::cerr << result.error.line << ":" << result.error.column
              << " — " << result.error.message << "\n";
    return 1;
}

amanuensis::core::Value root = result.value;
```

```cpp
auto result = amanuensis::json::Reader::ParseFile("config.json");
```

### Writing

`Writer` has two static methods. `WriteToFile` returns `bool` rather than throwing on I/O failure.

```cpp
using amanuensis::core::Value;

Value root = Value::MakeObject();
root.Insert("version", Value{1LL});
root.Insert("name", Value{std::string("example")});

// Pretty-printed (default)
std::string text = amanuensis::json::Writer::WriteToString(root);

// Minified
amanuensis::json::WriterOptions options;
options.pretty = false;
std::string minified = amanuensis::json::Writer::WriteToString(root, options);

// Write to disk
bool ok = amanuensis::json::Writer::WriteToFile(root, "output.json");
```

### The Value type

`core::Value` holds null, a boolean, an integer (`long long`), a double, a string, an array or an object. Objects keep their insertion order. Its operations are member functions.

```cpp
using amanuensis::core::Value;

// Construction
Value null_value;                          // null
Value boolean{true};
Value integer{42LL};
Value number{3.14};
Value text{std::string("hello")};
Value array = Value::MakeArray();
Value object = Value::MakeObject();

// Type inspection
value.GetType();     // amanuensis::core::ValueType
value.IsNull();
value.IsBoolean();
value.IsInteger();
value.IsDouble();
value.IsNumber();    // true for Integer or Double
value.IsString();
value.IsArray();
value.IsObject();

// Typed accessors — throw amanuensis::core::TypeMismatchError on wrong type
bool        b = value.AsBoolean();
long long   i = value.AsInteger();
double      d = value.AsDouble();
std::string s = value.AsString();

// Array operations
array.PushBack(Value{99LL});
std::size_t count = array.Size();
Value& element = array.At(0);           // throws IndexOutOfRangeError if out of range

// Object operations — insertion order is preserved
object.Insert("key", Value{std::string("value")});
bool exists = object.Contains("key");
Value& v = object.Get("key");           // throws KeyNotFoundError if absent
const Value* p = object.Find("key");    // nullptr if absent
for (auto entry = object.BeginObject(); entry != object.EndObject(); ++entry) {
    // entry->first is the key, entry->second the value
}
```

### Merging, comparing and hashing values

`amanuensis::core` has three format-neutral operations on whole values:

```cpp
using amanuensis::core::Value;

// Objects merge key by key; arrays and everything else replace whole.
Value entity = amanuensis::core::Overlay(prefab, overrides);

// The key paths where entity differs from prefab, such as {"position", "x"}.
std::vector<amanuensis::core::KeyPath> changed = amanuensis::core::Diff(prefab, entity);

// FNV-1a 64 over a fixed encoding: the same on every compiler, and key order doesn't matter.
std::uint64_t hash = amanuensis::core::StableHash(prefab);
```

### Reading Calamus

Calamus is a line-based entity format with one field per line, where a `*` in the first column marks the fields that override the entity's base. [docs/calamus-spec.md](docs/calamus-spec.md) defines it. Reading happens in two steps, because the header names the base and the caller has to resolve it:

```cpp
auto read = amanuensis::calamus::Reader::ParseFile("entities/door_01.calamus");
if (!read.succeeded) {
    std::cerr << read.error.line << ":" << read.error.column << " — " << read.error.message << "\n";
    return 1;
}

// @version, @type, and optionally @id, @base and @base_hash.
const amanuensis::calamus::Header& header = read.document.header;
amanuensis::core::Value prefab = FindPrefab(header.base, header.type);  // the caller's own lookup

auto loaded = amanuensis::calamus::Reader::Load(read.document, prefab);
amanuensis::core::Value entity = loaded.value;  // the prefab with the marked fields applied
```

A file older than its type's current version is migrated between the two steps. Only its overrides are upgraded, and everything else follows the current base:

```cpp
amanuensis::calamus::Migration migration{amanuensis::CurrentVersion<Door>, amanuensis::MigrateStep<Door>};
auto migrated = amanuensis::calamus::Migrate(read.document, migration);  // fails on a file newer than the code
```

`Load` never writes. When a person has edited the file by hand, for example by changing a value without adding its `*`, it sets `needsRewrite` and lists the field under `promoted` or `reset`. The caller can then regenerate the text and write it if it changed. `amanuensis::calamus::Heal(path, base)` does exactly that for editor tooling (never in a shipped build). Changes to the base are reported as `refreshed`, `stale` and `moved`, and they wait for the next save.

### Writing Calamus

`calamus::Writer` takes the header, the entity and its base. It works out the `*` marks and the recorded defaults itself, and sets `@base_hash` from the base:

```cpp
amanuensis::calamus::Header header;
header.version = 3;
header.type = "Door";
header.id = 0x3f9c2a7be1d04c58;
header.base = "prefabs/wooden_door";

auto written = amanuensis::calamus::Writer::WriteToString(header, entity, prefab);
if (!written.succeeded) {
    std::cerr << written.error << "\n";  // such as NaN in a field, which Calamus can't write
}
```

Some objects only make sense as a whole, such as a quaternion. Declare their types atomic, and the writer marks every component when any one of them differs, so a prefab change can't mix new and old components:

```cpp
AMANUENSIS_SERIALISABLE(Quaternion, x, y, z, w);
AMANUENSIS_ATOMIC(Quaternion);

amanuensis::calamus::WriterOptions options;
options.atomic = amanuensis::AtomicPaths(door);  // where ToValue(door) puts each atomic object
auto written = amanuensis::calamus::Writer::WriteToString(
    header, amanuensis::ToValue(door), amanuensis::ToValue(prefab), options);
```

Pass the same options to `Heal`. The same input always gives the same text, so a caller healing a file compares `written.text` with the file on disk and writes only if they differ. Calamus can't remove a field, so keys that only the base has are left out, and they come back from the base on the next load.

---

## User-type Serialisation

Amanuensis provides `ToValue<T>` and `FromValue<T>` for user types that opt in via one of three mechanisms.

### Mechanism 1 — `AMANUENSIS_SERIALISABLE` macro (recommended for most types)

One line per type. Field names are used as-is for both the C++ identifier and the JSON key.

```cpp
struct PerFunctionCoverage {
    std::string qualifiedName;
    int startLine;
    int endLine;
    int linesTotal;
    int linesCovered;
    int executionCount;
};

AMANUENSIS_SERIALISABLE(
    PerFunctionCoverage,
    qualifiedName, startLine, endLine,
    linesTotal, linesCovered, executionCount
);
```

Both directions then work automatically:

```cpp
// Serialise
PerFunctionCoverage pfc = { "math::Add", 10, 14, 5, 5, 3 };
amanuensis::core::Value v = amanuensis::ToValue(pfc);
amanuensis::json::Writer::WriteToFile(v, "coverage.json");

// Deserialise
auto result = amanuensis::json::Reader::ParseFile("coverage.json");
PerFunctionCoverage roundTripped = amanuensis::FromValue<PerFunctionCoverage>(result.value);

// Non-throwing variant
auto tryResult = amanuensis::TryFromValue<PerFunctionCoverage>(result.value);
if (!tryResult.succeeded) {
    std::cerr << tryResult.errorMessage << "\n";
}
```

Nested types and `std::vector<T>`, `std::optional<T>`, and `std::map<std::string, T>` are supported out of the box as long as the element type has also opted in.

> **Note:** Bare commas in macro arguments cause preprocessor issues. Template types (e.g. `std::map<K, V>`) and braced initialisers inside `AMANUENSIS_SERIALISABLE` arguments should use `using` aliases or file-scope factory functions as workarounds.

### Mechanism 2 — Intrusive `Serialise` member

For types that need custom JSON key names, computed fields, or versioning logic. The same method handles both read and write directions.

```cpp
struct RenamedFields {
    std::string name;
    int count;

    template <typename Archive>
    void Serialise(Archive& archive) {
        archive.Field("display_name", name);
        archive.Field("item_count", count);
    }
};
```

### Mechanism 3 — `SerialTraits<T>` specialisation

For types you do not own (external types), or for types that require a non-object JSON representation.

```cpp
namespace amanuensis {
template <> struct SerialTraits<Vec3> {
    static core::Value ToValue(const Vec3& v) {
        core::Value array = core::Value::MakeArray();
        array.PushBack(core::Value{v.x});
        array.PushBack(core::Value{v.y});
        array.PushBack(core::Value{v.z});
        return array;
    }
    static Vec3 FromValue(const core::Value& value) {
        return { value.At(0).AsDouble(),
                 value.At(1).AsDouble(),
                 value.At(2).AsDouble() };
    }
};
} // namespace amanuensis
```

### Data versions and migration

When a type's layout changes, such as a renamed field or a new unit, raise its version and add a step that upgrades old data by one version. A type without `AMANUENSIS_VERSION` is at version 1.

```cpp
AMANUENSIS_SERIALISABLE(Door, position, angle);
AMANUENSIS_VERSION(Door, 3);

// Called once per step: fromVersion -> fromVersion + 1.
void AmanuensisMigrate(Door*, amanuensis::core::Value& value, int fromVersion)
{
    if (fromVersion == 1 && value.Contains("pos")) {
        value.Insert("position", value.Get("pos"));
        value.Erase("pos");
    }
    if (fromVersion == 2 && value.Contains("angleDegrees")) {
        double degrees = value.Get("angleDegrees").AsDouble();
        value.Insert("angle", amanuensis::core::Value{degrees * std::numbers::pi / 180.0});
        value.Erase("angleDegrees");
    }
}

// Upgrade data that was saved at version 1, then deserialise it.
auto migrated = amanuensis::Migrate<Door>(value, 1);  // never throws; check migrated.succeeded
Door door = amanuensis::FromValue<Door>(value);
```

Data newer than the code is refused. A Calamus file records its version in `@version`, and `calamus::Migrate` upgrades only its overrides (see [Reading Calamus](#reading-calamus)).

---

## Testing

Tests are written using [Cimmerian](https://github.com/DeanWilsonDev/Cimmerian), a first-party BDD-style C++ testing framework. This is not a circular build dependency — `libamanuensis` has no project dependencies. Only the `test_amanuensis` binary links against Cimmerian.

---

## Requirements

- C++26
- GCC 14+ / Clang 19+ / a recent AppleClang (configure fails on an older GCC or Clang)
- On Windows, clang-cl from Clang 19+; MSVC's cl.exe isn't supported
- CMake 3.30+
