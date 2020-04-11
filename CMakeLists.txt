project(messenger)
cmake_minimum_required(VERSION 3.0)

set (BOOST_ROOT ./boost)

set(Boost_USE_STATIC_LIBS       OFF)  # only find static libs
set(Boost_USE_DEBUG_LIBS        OFF) # ignore debug libs
set(Boost_USE_RELEASE_LIBS      OFF)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

add_definitions(-std=c++17)

find_package(Boost)
find_package(Threads)
if(Boost_FOUND AND Threads_FOUND)
    message(STATUS "Success!")
    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(server chat_server.cpp chat_server.h chat_message.cpp  chat_message.h)
    add_executable(client chat_client.cpp chat_client.h chat_message.cpp  chat_message.h)

    target_link_libraries(server pthread)
    target_link_libraries(client pthread)

    # create chat_message library to use it with gtest
    add_library(messenger chat_message.cpp  chat_message.h)
else()
    message(STATUS "Fail!")
endif()

add_subdirectory(messenger_tests)
