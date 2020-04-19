project(messenger_tests)
cmake_minimum_required(VERSION 3.0)

add_subdirectory(googletest)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(messenger_tests_run chat_message_test.cpp)

target_link_libraries(messenger_tests_run messenger)
target_link_libraries(messenger_tests_run gtest gtest_main gmock)
