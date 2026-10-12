cmake_minimum_required(VERSION 3.30)

if(TARGET amanuensis)
  return()
endif()

project(
  amanuensis
  VERSION 1.0.0
  LANGUAGES CXX)

if((CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
   OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19))
  message(FATAL_ERROR "Amanuensis needs GCC 14 or later or Clang 19 or later; found ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
endif()

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(STATUS "AMANUENSIS_IS_ROOT_PROJECT is ON")
  set(AMANUENSIS_IS_ROOT_PROJECT ON)
else()
  set(AMANUENSIS_IS_ROOT_PROJECT OFF)
endif()

# ------------------------
# Library target
# -------------------------
add_library(
  amanuensis STATIC
  src/calamus/canonical.cpp
  src/calamus/heal.cpp
  src/calamus/migrate.cpp
  src/calamus/parser/body/body-line-parser.cpp
  src/calamus/parser/body/item-line-parser.cpp
  src/calamus/parser/body/key-line-parser.cpp
  src/calamus/parser/body/line-end-parser.cpp
  src/calamus/parser/body/line-prefix-parser.cpp
  src/calamus/parser/body/tree-builder.cpp
  src/calamus/parser/header/header-field-table.cpp
  src/calamus/parser/header/header-parser.cpp
  src/calamus/parser/header/hex.cpp
  src/calamus/parser/header/parselets/base-field-parselet.cpp
  src/calamus/parser/header/parselets/base-hash-field-parselet.cpp
  src/calamus/parser/header/parselets/id-field-parselet.cpp
  src/calamus/parser/header/parselets/type-field-parselet.cpp
  src/calamus/parser/header/parselets/version-field-parselet.cpp
  src/calamus/parser/line-cursor.cpp
  src/calamus/parser/parser.cpp
  src/calamus/parser/source-text.cpp
  src/calamus/parser/values/parselets/inline-array-parselet.cpp
  src/calamus/parser/values/parselets/number-parselet.cpp
  src/calamus/parser/values/parselets/quoted-string-parselet.cpp
  src/calamus/parser/values/parselets/rejected-value-parselet.cpp
  src/calamus/parser/values/parselets/word-parselet.cpp
  src/calamus/parser/values/value-parselet-table.cpp
  src/calamus/parser/values/value-parser.cpp
  src/calamus/reader.cpp
  src/calamus/writer/body-writer.cpp
  src/calamus/writer/changes.cpp
  src/calamus/writer/header-writer.cpp
  src/calamus/writer/key-writer.cpp
  src/calamus/writer/value-writer.cpp
  src/calamus/writer/writer.cpp
  src/core/converter.cpp
  src/core/diff.cpp
  src/core/migrate.cpp
  src/core/number-format.cpp
  src/core/overlay.cpp
  src/core/quoted-string/escape.cpp
  src/core/quoted-string/read-quoted-string.cpp
  src/core/quoted-string/unicode-escape.cpp
  src/core/quoted-string/utf8.cpp
  src/core/quoted-string/write-quoted-string.cpp
  src/core/stable-hash.cpp
  src/core/value.cpp
  src/core/ordered-map.cpp
  src/core/value-traits.cpp
  src/json/reader.cpp
  src/json/writer.cpp
  src/json/writer-context.cpp
  src/json/parser/parselets/array-parselet.cpp
  src/json/parser/parselets/literal-parselet.cpp
  src/json/parser/parselets/number-parselet.cpp
  src/json/parser/parselets/object-parselet.cpp
  src/json/parser/parselets/string-parselet.cpp
  src/json/parser/parser.cpp
  src/json/parser/value-parselet-table.cpp
  src/json/parser/value-parser.cpp
  src/json/parser/whitespace.cpp
  src/serialization/serialization.cpp)

target_include_directories(
  amanuensis PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:include>)

target_include_directories(amanuensis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(amanuensis PUBLIC cxx_std_26)
set_target_properties(amanuensis PROPERTIES C_EXTENSIONS OFF)

# -------------------------
# Output dirs (optional)
# -------------------------
set_target_properties(
  amanuensis
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(OUTPUTCONFIG Debug Release RelWithDebInfo MinSizeRel)
  string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
  set_target_properties(
    amanuensis
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
               "${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG}"
               LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
               "${CMAKE_BINARY_DIR}/lib/${OUTPUTCONFIG}"
               RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}
               "${CMAKE_BINARY_DIR}/bin/${OUTPUTCONFIG}")
endforeach()

# -------------------------
# Warnings
# -------------------------
if(MSVC)
  target_compile_options(amanuensis PRIVATE /W4)
else()
  target_compile_options(amanuensis PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -------------------------
# Build define for shared
# -------------------------
if(BUILD_SHARED_LIBS)
  target_compile_definitions(amanuensis PRIVATE AMANUENSIS_BUILD_DLL)
endif()

# -------------------------
# Debug Option
# -------------------------

option(ENABLE_DEBUG "Compile in extra Debug logging" OFF)

if(ENABLE_DEBUG)
  add_definitions(-DENABLE_DEBUG)
endif()

# -------------------------
# Test executable
# -------------------------

option(AMANUENSIS_BUILD_TESTS "Build amanuensis tests"
       ${AMANUENSIS_IS_ROOT_PROJECT})

if(AMANUENSIS_BUILD_TESTS)
  add_subdirectory(external/cimmerian)
  add_executable(
    test_amanuensis
    test/test-main.cpp
    test/reader.test.cpp
    test/writer.test.cpp
    test/round-trip.test.cpp
    test/value.test.cpp
    test/converter.test.cpp
    test/compat.test.cpp
    test/core-text.test.cpp
    test/value-utilities.test.cpp
    test/calamus/atomic/amanuensis-atomic.test.cpp
    test/calamus/atomic/atomic-paths.test.cpp
    test/calamus/atomic/loading.test.cpp
    test/calamus/atomic/writing.test.cpp
    test/calamus/conformance/hole-01-silent-hand-edits.test.cpp
    test/calamus/conformance/hole-02-one-item-list.test.cpp
    test/calamus/conformance/hole-03-integer-and-double-drift.test.cpp
    test/calamus/conformance/hole-04-float-noise.test.cpp
    test/calamus/conformance/hole-05-arrays-of-objects.test.cpp
    test/calamus/conformance/hole-06-keys-that-arent-identifiers.test.cpp
    test/calamus/conformance/hole-07-separators-inside-strings.test.cpp
    test/calamus/conformance/hole-08-stale-unmarked-fields.test.cpp
    test/calamus/conformance/hole-09-empty-objects.test.cpp
    test/calamus/conformance/hole-10-comments-dont-survive-a-save.test.cpp
    test/calamus/conformance/hole-11-small-things.test.cpp
    test/calamus/conformance/round-trips.test.cpp
    test/calamus/heal.test.cpp
    test/calamus/load/canonical-numbers.test.cpp
    test/calamus/load/marked-fields.test.cpp
    test/calamus/load/the-whole-entity.test.cpp
    test/calamus/load/unmarked-fields.test.cpp
    test/calamus/reader/blocks.test.cpp
    test/calamus/reader/comments.test.cpp
    test/calamus/reader/fields.test.cpp
    test/calamus/reader/header.test.cpp
    test/calamus/reader/marker-column-and-indentation.test.cpp
    test/calamus/reader/recorded-defaults.test.cpp
    test/calamus/reader/spec-example.test.cpp
    test/calamus/reader/text.test.cpp
    test/calamus/reader/values.test.cpp
    test/calamus/writer/formatting.test.cpp
    test/calamus/writer/header.test.cpp
    test/calamus/writer/marks.test.cpp
    test/calamus/writer/numbers.test.cpp
    test/calamus/writer/round-trips.test.cpp
    test/calamus/writer/writer.test.cpp
    test/migration/amanuensis-version.test.cpp
    test/migration/calamus-documents.test.cpp
    test/migration/core-migrate.test.cpp
    test/migration/heal.test.cpp
    test/migration/value-erase.test.cpp
    test/serialization/built-in-floating-point.test.cpp
    test/serialization/built-in-map.test.cpp
    test/serialization/built-in-optional.test.cpp
    test/serialization/built-in-vector.test.cpp
    test/serialization/extra-fields.test.cpp
    test/serialization/file-round-trip.test.cpp
    test/serialization/mechanism-1-serialisable-macro.test.cpp
    test/serialization/mechanism-2-serialise-member.test.cpp
    test/serialization/mechanism-3-serial-traits.test.cpp
    test/serialization/optional-fields.test.cpp
    test/serialization/try-from-value.test.cpp
  )
  target_link_libraries(test_amanuensis PRIVATE amanuensis cimmerian)
  target_include_directories(
    test_amanuensis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/test
  )
  target_compile_features(test_amanuensis PRIVATE cxx_std_26)
  # No -Wpedantic: Cimmerian's test macros use __COUNTER__, which clang 22
  # reports as a C2y extension in every test.
  if(MSVC)
    target_compile_options(test_amanuensis PRIVATE /W4)
  else()
    target_compile_options(test_amanuensis PRIVATE -Wall -Wextra)
  endif()
endif()

# ------------------------
# Installation
# -------------------------
include(GNUInstallDirs)

install(
  TARGETS amanuensis
  EXPORT amanuensisTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  EXPORT amanuensisTargets
  FILE amanuensisTargets.cmake
  NAMESPACE amanuensis::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/amanuensis)
