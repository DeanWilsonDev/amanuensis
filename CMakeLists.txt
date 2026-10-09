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
  src/core/converter.cpp
  src/core/diff.cpp
  src/core/number-format.cpp
  src/core/overlay.cpp
  src/core/quoted-string.cpp
  src/core/stable-hash.cpp
  src/core/ordered-map.cpp
  src/core/value-traits.cpp
  src/json.cpp
  src/json/reader.cpp
  src/json/writer.cpp
  src/json/writer-context.cpp
  src/json/parser.cpp
  src/serialization/serialization.cpp)

target_include_directories(
  amanuensis PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:include>)

# The formats share internal helpers in src/core/, included as "core/...".
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
    test/test-main.cpp test/reader.test.cpp test/writer.test.cpp
    test/round-trip.test.cpp test/value.test.cpp test/serialization.test.cpp
    test/converter.test.cpp test/compat.test.cpp test/core-text.test.cpp
    test/value-utilities.test.cpp)
  target_link_libraries(test_amanuensis PRIVATE amanuensis cimmerian)
  # The tests reach the formats' shared helpers in src/core/ directly.
  target_include_directories(test_amanuensis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  target_compile_features(test_amanuensis PRIVATE cxx_std_26)
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
