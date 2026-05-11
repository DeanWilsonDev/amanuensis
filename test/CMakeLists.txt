set(TEST_TARGET_NAME test_amanuensis)
add_executable(
  ${TEST_TARGET_NAME} test-main.cpp reader.test.cpp writer.test.cpp
                      round-trip.test.cpp value.test.cpp serialisation.test.cpp)


target_link_libraries(${TEST_TARGET_NAME} PRIVATE amanuensis cimmerian)

if(MSVC)
  target_compile_options(${TEST_TARGET_NAME} PRIVATE /W4 /EHsc)
else()
  target_compile_options(${TEST_TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
