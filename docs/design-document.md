# Amanuensis — Design Document

> A zero-dependency C++20 JSON read/write library. Minimal feature surface, insertion-order preserving, consumed via CMake `add_subdirectory`.
> 

---

## Overview

Amanuensis is a first-party C++20 library that reads and writes JSON. It exists to replace `nlohmann/json` as a third-party dependency across the project ecosystem (Cimmerian, Prism, and anything that follows them), consolidating JSON handling into one well-understood, owned piece of code.

The library is deliberately minimal. It covers the subset of JSON that real first-party projects actually use, and nothing more. Features that would be nice in a general-purpose library — schema validation, streaming parsers, binary formats — are explicit non-goals. User-type serialisation is supported through a tiered opt-in system (macro, intrusive method, or traits specialisation) rather than through runtime reflection, which C++ does not support.

---

## Goals

- Read and write JSON conforming to RFC 8259
- Preserve insertion order on write so generated files are stable under diffing
- Zero external dependencies — C++20 stdlib only
- Consumed via CMake `add_subdirectory`, matching the Cimmerian integration pattern
- Produce human-readable output by default (pretty-printed, 2-space indent)
- Provide a minified output mode for wire formats and size-sensitive use cases
- Clear, explicit error reporting on parse failure — line, column, and reason
- Low-boilerplate serialisation of user types via a tiered system: a one-line macro for the common case, an intrusive method for full control, and a traits specialisation for external types

## Non-Goals

- JSON Schema validation
- Streaming / incremental parsing
- Binary formats (BSON, CBOR, MessagePack)
- Runtime-reflection-based automatic serialisation (C++ does not support it; C++26 static reflection may make this viable in the future)
- JSON5, JSONC, or other relaxed dialects — Amanuensis is strict RFC 8259
- Performance parity with SIMD-optimised parsers like simdjson

---

## Relationship to Other Projects

Amanuensis is a dependency of Cimmerian and Prism, not the other way around. Both consumers pull Amanuensis in the same way: as a git subdirectory under `libs/`, linked via CMake `add_subdirectory`.

```
amanuensis  ──produced by─►  libamanuensis.a
                                 │
       ┌─────────────────────────┴───────────────────────┐
       ▼                                                  ▼
cimmerian (coverage.json out)             prism (compile_commands.json in,
                                                  analysis.json out)
```

Amanuensis itself has no dependencies beyond the C++20 standard library. Its own tests are written using Cimmerian — this is fine and is not a circular build dependency, because the test binary is a separate target from the library itself. Amanuensis the library compiles cleanly with zero project dependencies; only `amanuensis_tests` links against Cimmerian.

---

## Public API

The entire public surface lives in a single namespace `Amanuensis` and consists of four headers: the data model (`value.hpp`), the reader (`reader.hpp`), the writer (`writer.hpp`), and the user-type serialisation layer (`serialisation.hpp`).

### `amanuensis/value.hpp` — the data model

```cpp
namespace Amanuensis {

enum class ValueType {
  Null,
  Boolean,
  Integer,
  Double,
  String,
  Array,
  Object,
};

class Value {
public:
  // Construction
  Value();                               // Null
  Value(std::nullptr_t);                 // Null
  Value(bool);                           // Boolean
  Value(int);                            // Integer
  Value(long long);                      // Integer
  Value(double);                         // Double
  Value(const char*);                    // String
  Value(std::string);                    // String
  static Value MakeArray();
  static Value MakeObject();

  // Type inspection
  ValueType GetType() const;
  bool IsNull() const;
  bool IsBoolean() const;
  bool IsInteger() const;
  bool IsDouble() const;
  bool IsNumber() const;                 // true for Integer or Double
  bool IsString() const;
  bool IsArray() const;
  bool IsObject() const;

  // Typed accessors — throw Amanuensis::TypeMismatchError on wrong type
  bool          AsBoolean() const;
  long long     AsInteger() const;
  double        AsDouble() const;
  const std::string& AsString() const;

  // Array operations
  void PushBack(Value value);
  std::size_t Size() const;              // works for Array or Object
  const Value& At(std::size_t index) const;
  Value&       At(std::size_t index);

  // Object operations
  void Insert(std::string key, Value value);         // preserves insertion order
  bool Contains(const std::string& key) const;
  const Value& Get(const std::string& key) const;    // throws if absent
  Value&       Get(const std::string& key);
  const Value* Find(const std::string& key) const;   // returns nullptr if absent

  // Ordered iteration for Object — yields keys in insertion order
  class ObjectIterator;
  ObjectIterator BeginObject() const;
  ObjectIterator EndObject() const;

  // Array iteration
  const std::vector<Value>& AsArray() const;
};

}  // namespace Amanuensis
```

### `amanuensis/reader.hpp` — parsing

```cpp
namespace Amanuensis {

struct ParseError {
  std::string message;
  int line;
  int column;
};

struct ParseResult {
  bool succeeded;
  Value value;                    // meaningful only when succeeded == true
  ParseError error;               // meaningful only when succeeded == false
};

class Reader {
public:
  static ParseResult ParseString(std::string_view text);
  static ParseResult ParseFile(const std::filesystem::path& path);
};

}  // namespace Amanuensis
```

The reader returns a result struct rather than throwing so that callers like Prism, which run against large codebases where a single malformed file should not abort the whole pipeline, can continue on parse errors with a logged warning.

### `amanuensis/writer.hpp` — serialisation

```cpp
namespace Amanuensis {

struct WriterOptions {
  bool pretty = true;
  int indentWidth = 2;
  char indentChar = ' ';
  bool trailingNewline = true;
};

class Writer {
public:
  static std::string WriteToString(const Value& value, const WriterOptions& options = {});
  static bool WriteToFile(const Value& value, const std::filesystem::path& path, const WriterOptions& options = {});
};

}  // namespace Amanuensis
```

`WriteToFile` returns a bool rather than throwing on I/O failure, matching the Reader's non-throwing convention.

### `amanuensis/serialisation.hpp` — user-type serialisation

The serialisation layer provides `ToJson<T>` and `FromJson<T>` for any user type `T` that has opted in via one of three mechanisms. All three route through the same `Archive` abstraction internally, so from the library's point of view there is one mechanism with three entry points.

```cpp
namespace Amanuensis {

// Top-level conversion functions — work for any T that has opted in
// via any of the three mechanisms below.
template <typename T> Value ToJson(const T& value);
template <typename T> T     FromJson(const Value& value);

// FromJson variant that returns a result struct for types where
// malformed input should not throw.
template <typename T>
struct FromJsonResult {
  bool succeeded;
  T value;
  std::string errorMessage;
};
template <typename T> FromJsonResult<T> TryFromJson(const Value& value);

}  // namespace Amanuensis
```

#### Mechanism 1 — `AMANUENSIS_SERIALISABLE` macro (the default path)

One line per type. The macro expands to a `Serialise` free function that visits each named field using the same name as both the C++ identifier and the JSON key.

```cpp
struct PerFunctionCoverage {
  std::string qualifiedName;
  int startLine;
  int endLine;
  int linesTotal;
  int linesCovered;
  int executionCount;
};

AMANUENSIS_SERIALISABLE(PerFunctionCoverage,
  qualifiedName, startLine, endLine,
  linesTotal, linesCovered, executionCount);
```

Once the macro is in place, both directions work directly:

```cpp
// Serialise an object to a Value, then write to disk
PerFunctionCoverage pfc = { "math::Add", 10, 14, 5, 5, 3 };
Amanuensis::Value v = Amanuensis::ToJson(pfc);
Amanuensis::Writer::WriteToFile(v, "coverage.json");

// Read from disk, then deserialise into an object
auto parseResult = Amanuensis::Reader::ParseFile("coverage.json");
if (!parseResult.succeeded) {
  std::cerr << "parse failed at " << parseResult.error.line
            << ":" << parseResult.error.column
            << ": " << parseResult.error.message << "\n";
  return 1;
}
PerFunctionCoverage roundTripped = Amanuensis::FromJson<PerFunctionCoverage>(parseResult.value);

// For types where malformed input should not throw, use TryFromJson:
auto tryResult = Amanuensis::TryFromJson<PerFunctionCoverage>(parseResult.value);
if (!tryResult.succeeded) {
  std::cerr << "conversion failed: " << tryResult.errorMessage << "\n";
}
```

Nested types work without extra effort. A `PerFileCoverage` that contains a `std::vector<PerFunctionCoverage>` will round-trip in both directions as long as `PerFileCoverage` has also opted in via `AMANUENSIS_SERIALISABLE` — the library's built-in `JsonTraits<std::vector<T>>` specialisation handles the vector, which recursively invokes `FromJson<PerFunctionCoverage>` for each element.

This path is expected to cover the majority of types in first-party projects.

#### Mechanism 2 — Intrusive `Serialise` member

For types that need different JSON keys from their C++ field names, computed fields, conditional serialisation, or versioning logic, the type declares a `Serialise` member template. The same method handles both directions — the `Archive` parameter is either a reader or a writer and the `Field` call dispatches correctly.

```cpp
struct PerFunctionCoverage {
  std::string qualifiedName;
  int startLine;
  int endLine;
  int linesTotal;
  int linesCovered;
  int executionCount;

  template <typename Archive>
  void Serialise(Archive& ar) {
    ar.Field("qualifiedName", qualifiedName);
    ar.Field("start_line", startLine);       // snake_case in JSON
    ar.Field("end_line", endLine);
    ar.Field("lines_total", linesTotal);
    ar.Field("lines_covered", linesCovered);
    ar.Field("execution_count", executionCount);
  }
};
```

This pattern is borrowed from the `cereal` library. The cost is roughly the same as writing `ToJson`/`FromJson` by hand, but collapsed into a single function with a single list of fields.

#### Mechanism 3 — `JsonTraits` specialisation (for types you don't own)

When the type cannot be modified — a struct from a third-party library, a type generated by another tool — serialisation is opted in from the outside by specialising `JsonTraits`.

```cpp
namespace Amanuensis {
template <>
struct JsonTraits<SomeExternalLib::Vec3> {
  static Value ToJson(const SomeExternalLib::Vec3& v) {
    auto arr = Value::MakeArray();
    arr.PushBack(v.x);
    arr.PushBack(v.y);
    arr.PushBack(v.z);
    return arr;
  }
  static SomeExternalLib::Vec3 FromJson(const Value& v) {
    return { v.At(0).AsDouble(), v.At(1).AsDouble(), v.At(2).AsDouble() };
  }
};
}  // namespace Amanuensis
```

This mechanism is also what the library itself uses internally to teach `ToJson` and `FromJson` about standard library types — `std::vector<T>`, `std::optional<T>`, `std::map<std::string, T>`, and so on are all handled via built-in `JsonTraits` specialisations. Consumers never need to write conversion functions for stdlib containers.

#### Resolution order

When a consumer calls either `ToJson<T>(value)` or `FromJson<T>(value)`, the library resolves which mechanism to use in this order:

1. A `JsonTraits<T>` specialisation, if one exists.
2. A `Serialise` member on `T`, if one exists.
3. A free `Serialise` function in `T`'s namespace (which is what the `AMANUENSIS_SERIALISABLE` macro generates).
4. Compile error with a message naming the type and listing the three opt-in mechanisms.

The resolution order is the same for both directions. A type that opts in via any mechanism gets serialisation *and* deserialisation from that single opt-in — you never need to register the type twice. This means a consumer can start with the macro, upgrade to the intrusive method if they hit its limits, and never touch `JsonTraits` unless they're integrating an external type.

#### Handling missing, extra, and mistyped fields

Deserialisation behaviour for common edge cases:

- **Missing required field in JSON**: throws `Amanuensis::MissingFieldError` (or, when using `TryFromJson`, returns `succeeded = false` with a message naming the field). A field is "required" if its C++ type is not `std::optional<T>`.
- **Missing optional field in JSON**: a field declared as `std::optional<T>` is left empty if the JSON key is absent or the value is `null`. Present values populate the optional.
- **Extra fields in JSON**: silently ignored. This is the correct default for schema evolution — consumers shouldn't break when the producer adds new fields. A strict mode that rejects unknown fields is listed as a future follow-up, not v1.
- **Type mismatch** (e.g., JSON has `"startLine": "10"` but the struct expects `int`): throws `Amanuensis::TypeMismatchError`, or returns a failed `TryFromJsonResult` with a message naming the field and both types.

---

## Insertion Order Preservation

Objects preserve the order in which keys were inserted. This applies both to objects built programmatically and to objects parsed from a source file — a round-trip read-then-write of a file preserves the order of its keys.

The implementation uses a `std::vector<std::pair<std::string, Value>>` for the object's backing store, with a parallel `std::unordered_map<std::string, std::size_t>` for O(1) key lookup. Insertion is amortised O(1); lookup is O(1); iteration is O(n) in insertion order.

This choice trades a small amount of memory per object for stable, predictable output. For the file sizes both Cimmerian and Prism produce (coverage reports, architecture analyses), the tradeoff is firmly in favour of predictable output — diffs in git should reflect real changes, not hash-order reshuffling.

---

## Number Handling

Amanuensis distinguishes between integer and floating-point numbers internally. On parse, a number literal without a decimal point or exponent is stored as `long long`; anything with `.`, `e`, or `E` is stored as `double`.

On write, integers are emitted without a decimal point; doubles are emitted with enough precision for a lossless round-trip (`std::format("{:.17g}", value)` or equivalent).

Overflow on parse (an integer literal larger than `LLONG_MAX`) falls back to `double` silently. Callers that care about this distinction should inspect `IsInteger()` vs `IsDouble()` on the parsed value.

---

## Error Reporting

Parse errors carry a message, a line number, and a column number. The message names the specific problem (for example, "unexpected character ',' after value", "unterminated string literal", "invalid escape sequence '\q'"). Line and column refer to the byte position in the input, counting from 1.

This is enough context for a caller to print a useful diagnostic like `compile_commands.json:42:18: unterminated string literal` without needing access to the input itself.

---

## Project Structure

```
amanuensis/
├── CMakeLists.txt
├── include/
│   └── amanuensis/
│       ├── amanuensis.hpp          — umbrella header, re-exports the four below
│       ├── value.hpp
│       ├── reader.hpp
│       ├── writer.hpp
│       └── serialisation.hpp      — ToJson / FromJson / JsonTraits / AMANUENSIS_SERIALISABLE
├── src/
│   ├── value.cpp
│   ├── reader.cpp
│   ├── writer.cpp
│   └── serialisation.cpp
├── test/
│   ├── test-main.cpp
│   ├── reader.test.cpp
│   ├── writer.test.cpp
│   ├── round-trip.test.cpp
│   ├── insertion-order.test.cpp
│   └── serialisation.test.cpp
└── libs/
    └── Cimmerian/                  — used by tests only
```

---

## Tech Stack

| Component | Technology |
| --- | --- |
| Language | C++20 |
| Build system | CMake |
| Testing | Cimmerian (test-only dependency) |
| Platform | Linux (primary), macOS, Windows |

---

## CMake Integration

Consumers pull Amanuensis in the same way they pull Cimmerian:

```
add_subdirectory(libs/amanuensis)

add_executable(prism ...)
target_link_libraries(prism PRIVATE amanuensis)
```

The library target is `amanuensis`. The umbrella header is `<amanuensis/amanuensis.hpp>`; individual headers can be included directly if preferred.

---

## Testing Strategy

Amanuensis is tested using Cimmerian. The test suite covers five categories, one per test file:

- **reader.test.cpp** — parsing correctness, including edge cases (deep nesting, escape sequences, numeric edge values, Unicode in strings) and error reporting (line/column accuracy on malformed input).
- **writer.test.cpp** — output correctness for each type, pretty vs minified output, numeric precision on round-trip.
- **round-trip.test.cpp** — for a corpus of known-good JSON files, parsing and re-emitting produces byte-identical output (modulo normalised whitespace in pretty mode).
- **insertion-order.test.cpp** — objects built programmatically and objects parsed from source both preserve key order through any number of round trips.
- **serialisation.test.cpp** — all three opt-in mechanisms (`AMANUENSIS_SERIALISABLE` macro, intrusive `Serialise` member, `JsonTraits` specialisation) correctly round-trip representative user types. Also verifies the resolution order and the compile-error message for types that have opted in through none of the three.

Parse-error tests use Cimmerian's `ASSERT_FALSE(result.succeeded)` followed by precise checks on `result.error.line` and `result.error.column`.

---

## Key Design Decisions

**Non-throwing public API for I/O.** Reader returns a `ParseResult` struct rather than throwing `ParseError`. Writer returns `bool` on file write. This matches the real consumer pattern — Prism runs against hundreds of files and cannot abort on a single malformed input. Internal helpers may throw; the public surface does not.

**Separate `Integer` and `Double` value types.** JSON has one number type but real consumers care about the distinction. Line counts, branch counts, and coverage totals are integers; percentages are doubles. Forcing everything through `double` loses precision for large integers (anything above 2^53) and muddies downstream code.

**Insertion order over lexicographic order.** Two objects that are semantically identical should produce byte-identical output if they were built the same way. This matters for git diffs, code review, and reproducible builds. The memory cost is small and the ergonomic win is large.

**Three tiers of serialisation opt-in, unified by an Archive abstraction.** The library supports three ways to make a user type serialisable: a one-line `AMANUENSIS_SERIALISABLE` macro (the default), an intrusive `Serialise` member method (for flexibility), and a `JsonTraits` specialisation (for external types). All three route through the same internal `Archive` type, so adding a new tier or migrating between them does not change the underlying mechanism. The macro tier is deliberately the shortest path because it covers the common case where the JSON key can match the C++ field name — which is most cases in first-party projects. Consumers who need more control upgrade to the method tier at the cost of a few more lines per type. External types use the traits tier, which is also how the library internally teaches `ToJson`/`FromJson` about standard-library containers so consumers never need to write conversions for `std::vector`, `std::optional`, and similar.

**Strict RFC 8259.** No comments, no trailing commas, no unquoted keys. Human-edited configs that need those features are better served by YAML or TOML, not by a relaxed JSON dialect that makes the library harder to reason about.

---

## Phased Rollout

### Phase 1 — Value model, writer, and serialisation layer (1–2 weeks)

- `Value` class with all types, construction, and type inspection
- Object insertion-order backing store
- `Writer` with pretty and minified modes
- `Archive` abstraction and write-direction implementation
- `ToJson<T>` entry point with resolution order (traits → method → free function → compile error)
- `AMANUENSIS_SERIALISABLE` macro
- Built-in `JsonTraits` specialisations for common stdlib types (`std::vector<T>`, `std::optional<T>`, `std::map<std::string, T>`, `std::string`, arithmetic types)
- Tests for writer output correctness, order preservation, and all three write-direction opt-in mechanisms

At the end of this phase, Amanuensis can be used by Cimmerian for the coverage JSON reporter — Cimmerian only needs the write path and will consume the library primarily through `AMANUENSIS_SERIALISABLE`.

### Phase 2 — Reader and read-direction serialisation (1–2 weeks)

- Recursive-descent parser with explicit line/column tracking
- All JSON types including nested objects and arrays
- String escape handling including `\u` sequences
- Numeric parsing with integer/double disambiguation
- Precise error reporting
- Read-direction of the `Archive` abstraction — the same `Serialise` function, macro expansion, and traits specialisations now work for `FromJson<T>` as well
- `TryFromJson<T>` non-throwing variant for consumers like Prism that cannot abort on malformed input
- Tests for parsing correctness, error reporting, round-trip stability, and all three read-direction opt-in mechanisms

At the end of this phase, Amanuensis is feature-complete for Prism's needs.

### Phase 3 — Integration and hardening (1 week)

- Cimmerian switches its coverage JSON reporter to Amanuensis
- Prism switches `analysis.json` output and `compile_commands.json` input to Amanuensis
- Any bugs surfaced by the two real consumers are fixed before calling v1 done
- Tagged v1.0.0 release in git

### Phase 4 — Follow-ups (future, not scoped)

- Streaming output writer for very large files (if `analysis.json` ever gets big enough to matter)
- Optional schema-style assertions (`value.RequireString("expected a string at $.run.compiler")`) to reduce boilerplate error handling in consumers
- Additional output modes (JSON Lines, for example)

---

## Open Questions

- **`std::optional` return type for `Find`**: `Find` currently returns `const Value*` for absence. Would `std::optional<std::reference_wrapper<const Value>>` be clearer, or is the pointer form more idiomatic for this kind of lookup? Leaning pointer for simplicity.
- **Move-only or copyable `Value`**: copying a deeply nested `Value` is potentially expensive. Making `Value` move-only forces callers to be deliberate, but makes the API harder to use. Leaning copyable, with the understanding that consumers can move when they care.
- **File reader encoding**: the library assumes UTF-8 input. Should it actively reject non-UTF-8, or silently pass through bytes and let the caller deal with it? Leaning strict — reject with a parse error on invalid UTF-8 sequences inside string literals.

---

## Success Criteria

- Amanuensis builds cleanly on GCC 13+ and Clang 16+ with `-Wall -Wextra -Wpedantic` and no warnings
- The test suite achieves ≥ 90% line coverage of `src/` (self-dogfooded via Cimmerian's coverage feature)
- Round-tripping `compile_commands.json` from a real project (Umbra Engine) and Cimmerian's own emitted `coverage.json` produces byte-identical output on a second write
- Cimmerian's `coverage.json` output is produced entirely through Amanuensis, with no hand-rolled JSON code remaining in the coverage subsystem
- Cimmerian's coverage structs (`PerFileCoverage`, `PerFunctionCoverage`, etc.) opt in via `AMANUENSIS_SERIALISABLE` — no hand-written `ToJson` functions
- Prism replaces its `nlohmann/json` dependency with Amanuensis without regression, using `AMANUENSIS_SERIALISABLE` for the majority of its graph and metric types and intrusive `Serialise` methods for the handful that need JSON-key renaming