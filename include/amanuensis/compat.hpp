#pragma once

// Temporary: keeps code written against the old names compiling while
// consumers move over. Every public header pulls this in, so consumers need no
// change until the sweep after Calamus lands. It covers:
//
// - the old `Amanuensis::` namespace spelling;
// - the old Json-prefixed names (JsonValue, JsonValueType, JsonParseResult,
//   JsonParseError and JsonTraits);
// - names that moved from `amanuensis` into `amanuensis::core` or
//   `amanuensis::json`, such as amanuensis::OrderedMap and amanuensis::Reader.
//
// The sweep removes this file, the AMANUENSIS_NO_COMPAT blocks in
// serialization/ (the JsonTraits fallback in SerialTraits, and the ToJson,
// FromJson and TryFromJson forwarders) and the forwarding headers left at the
// old paths: json-value.hpp, ordered-map.hpp, object-iterator.hpp, errors.hpp,
// value-traits.hpp, converter.hpp, io/ and serialization/json-traits*.hpp.
//
// A namespace alias can't be reopened, so `namespace Amanuensis { ... }` in a
// consumer (a forward declaration or a JsonTraits specialisation) still has to
// move to `namespace amanuensis`. Likewise a forward declaration of JsonValue
// has to become one of core::Value, and a ValueTraits specialisation has to
// move to `namespace amanuensis::core`, because GCC rejects specialising a
// template through a using-declaration (Clang accepts it). Define
// AMANUENSIS_NO_COMPAT to check that a consumer no longer depends on any old
// name or header path.

#ifndef AMANUENSIS_NO_COMPAT
namespace amanuensis::core {

enum class ValueType;
struct Value;
struct ParseError;
struct ParseResult;
template <typename TValue> class OrderedMap;
class ObjectIterator;
class TypeMismatchError;
class KeyNotFoundError;
class IndexOutOfRangeError;
template <typename TValue, typename TValueArray, typename TValueObject> struct ValueTraits;
template <typename SourceValue, typename TargetValue, typename TargetTraits, typename SourceTraits>
class Converter;

} // namespace amanuensis::core

namespace amanuensis::json {

class Reader;
class Writer;
struct WriterOptions;

} // namespace amanuensis::json

namespace amanuensis {

using JsonValueType = core::ValueType;
using JsonValue = core::Value;
using JsonParseError = core::ParseError;
using JsonParseResult = core::ParseResult;

using core::Converter;
using core::IndexOutOfRangeError;
using core::KeyNotFoundError;
using core::ObjectIterator;
using core::OrderedMap;
using core::TypeMismatchError;
using core::ValueTraits;

using json::Reader;
using json::Writer;
using json::WriterOptions;

// The old customisation point. Specialisations of it, with static ToJson and
// FromJson members, are picked up by the primary SerialTraits template.
template <typename T> struct JsonTraits;

} // namespace amanuensis

namespace Amanuensis = amanuensis; // NOLINT(misc-unused-alias-decls): used by consumers
#endif
