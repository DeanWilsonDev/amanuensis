#pragma once

// Temporary: keeps code written against the old `Amanuensis::` namespace and
// the old Json-prefixed names compiling while consumers move over. Every
// public header pulls this in, so consumers need no change until the sweep
// after Calamus lands. The sweep removes this file, the AMANUENSIS_NO_COMPAT
// blocks in serialization/ (the JsonTraits fallback in SerialTraits, and the
// ToJson, FromJson and TryFromJson forwarders) and the forwarding headers left
// at the old paths (json-value.hpp, io/json-parse-result.hpp,
// io/json-parse-error.hpp, serialization/json-traits.hpp and
// serialization/json-traits-std.hpp).
//
// A namespace alias can't be reopened, so `namespace Amanuensis { ... }` in a
// consumer (a forward declaration or a JsonTraits specialisation) still has to
// move to `namespace amanuensis`. Likewise a forward declaration of JsonValue
// has to become one of Value. Define AMANUENSIS_NO_COMPAT to check that a
// consumer no longer depends on any old name or header path.

#ifndef AMANUENSIS_NO_COMPAT
namespace amanuensis {

enum class ValueType;
struct Value;
struct ParseError;
struct ParseResult;

using JsonValueType = ValueType;
using JsonValue = Value;
using JsonParseError = ParseError;
using JsonParseResult = ParseResult;

// The old customisation point. Specialisations of it, with static ToJson and
// FromJson members, are picked up by the primary SerialTraits template.
template <typename T> struct JsonTraits;

} // namespace amanuensis

namespace Amanuensis = amanuensis; // NOLINT(misc-unused-alias-decls): used by consumers
#endif
