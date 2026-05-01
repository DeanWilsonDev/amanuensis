#pragma once

namespace Amanuensis {

// -----------------------------------------------------------------------
// AMANUENSIS_SERIALISABLE macro — Mechanism 1 (the default path)
//
// Expands to a free function that the resolution-order logic can find
// via ADL.  The function visits each named field using the C++ field
// name as the JSON key.
// -----------------------------------------------------------------------

#define AMANUENSIS_DETAIL_FIELD(archive, field) archive.Field(#field, instance.field);

#define AMANUENSIS_SERIALISABLE(TypeName, ...)                                                     \
  template <typename ArchiveType>                                                                  \
  void AmanuensisSerialiseFree(TypeName& instance, ArchiveType& archive)                           \
  {                                                                                                \
    AMANUENSIS_DETAIL_APPLY(archive, __VA_ARGS__)                                                  \
  }

// Helper: apply AMANUENSIS_DETAIL_FIELD to each argument via a FOR_EACH pattern.
// We support up to 32 fields — more than enough for any real struct.

#define AMANUENSIS_DETAIL_APPLY(archive, ...) AMANUENSIS_DETAIL_FOR_EACH(archive, __VA_ARGS__)

// Counting / dispatching macros
#define AMANUENSIS_DETAIL_FOR_EACH(archive, ...)                                                                                                                                                                                                                    \
  AMANUENSIS_DETAIL_EXPAND(                                                                                                                                                                                                                                         \
      AMANUENSIS_DETAIL_SELECT(__VA_ARGS__, FE_32, FE_31, FE_30, FE_29, FE_28, FE_27, FE_26, FE_25, FE_24, FE_23, FE_22, FE_21, FE_20, FE_19, FE_18, FE_17, FE_16, FE_15, FE_14, FE_13, FE_12, FE_11, FE_10, FE_9, FE_8, FE_7, FE_6, FE_5, FE_4, FE_3, FE_2, FE_1)( \
          archive, __VA_ARGS__                                                                                                                                                                                                                                      \
      )                                                                                                                                                                                                                                                             \
  )

#define AMANUENSIS_DETAIL_EXPAND(x) x

#define AMANUENSIS_DETAIL_SELECT(                                                                  \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,     \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, NAME, ...                          \
)                                                                                                  \
  NAME

#define FE_1(ar, a) ar.Field(#a, instance.a);
#define FE_2(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_1(ar, __VA_ARGS__))
#define FE_3(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_2(ar, __VA_ARGS__))
#define FE_4(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_3(ar, __VA_ARGS__))
#define FE_5(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_4(ar, __VA_ARGS__))
#define FE_6(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_5(ar, __VA_ARGS__))
#define FE_7(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_6(ar, __VA_ARGS__))
#define FE_8(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_7(ar, __VA_ARGS__))
#define FE_9(ar, a, ...)                                                                           \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_8(ar, __VA_ARGS__))
#define FE_10(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_9(ar, __VA_ARGS__))
#define FE_11(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_10(ar, __VA_ARGS__))
#define FE_12(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_11(ar, __VA_ARGS__))
#define FE_13(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_12(ar, __VA_ARGS__))
#define FE_14(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_13(ar, __VA_ARGS__))
#define FE_15(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_14(ar, __VA_ARGS__))
#define FE_16(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_15(ar, __VA_ARGS__))
#define FE_17(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_16(ar, __VA_ARGS__))
#define FE_18(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_17(ar, __VA_ARGS__))
#define FE_19(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_18(ar, __VA_ARGS__))
#define FE_20(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_19(ar, __VA_ARGS__))
#define FE_21(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_20(ar, __VA_ARGS__))
#define FE_22(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_21(ar, __VA_ARGS__))
#define FE_23(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_22(ar, __VA_ARGS__))
#define FE_24(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_23(ar, __VA_ARGS__))
#define FE_25(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_24(ar, __VA_ARGS__))
#define FE_26(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_25(ar, __VA_ARGS__))
#define FE_27(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_26(ar, __VA_ARGS__))
#define FE_28(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_27(ar, __VA_ARGS__))
#define FE_29(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_28(ar, __VA_ARGS__))
#define FE_30(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_29(ar, __VA_ARGS__))
#define FE_31(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_30(ar, __VA_ARGS__))
#define FE_32(ar, a, ...)                                                                          \
  ar.Field(#a, instance.a);                                                                        \
  AMANUENSIS_DETAIL_EXPAND(FE_31(ar, __VA_ARGS__))

} // namespace Amanuensis
