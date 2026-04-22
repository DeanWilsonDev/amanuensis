cmake_minimum_required(VERSION 3.25)

project(
  amanuensis
  VERSION 1.0.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------------------
# Library target
# -------------------------
add_library(
  amanuensis STATIC src/value.cpp src/io/reader.cpp src/io/writer.cpp
                    src/io/parser.cpp src/serialisation/serialisation.cpp)

target_include_directories(
  amanuensis PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:include>)

target_compile_features(amanuensis PUBLIC cxx_std_20)
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
option(amanuensis_BUILD_TESTS "Build amanuensis tests" ON)

if(amanuensis_BUILD_TESTS)

  find_package(Cimmerian 1.0 REQUIRED)

  add_executable(
    test_amanuensis
    test/test-main.cpp test/reader.test.cpp test/writer.test.cpp
    test/round-trip.test.cpp test/value.test.cpp test/serialisation.test.cpp)

  target_link_libraries(test_amanuensis PRIVATE amanuensis Cimmerian::cimmerian)
  target_compile_features(test_amanuensis PRIVATE cxx_std_20)

  if(MSVC)
    target_compile_options(test_amanuensis PRIVATE /W4 /EHsc)
  else()
    target_compile_options(test_amanuensis PRIVATE -Wall -Wextra -Wpedantic)
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
