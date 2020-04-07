project(chat)
cmake_minimum_required(VERSION 3.0)

include_directories(boost)

add_definitions(-std=c++17)

add_executable(server chat_server.cpp)
add_executable(client chat_client.cpp)

target_link_libraries(server pthread)
target_link_libraries(client pthread)
