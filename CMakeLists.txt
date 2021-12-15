cmake_minimum_required(VERSION 3.17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

project(network LANGUAGES C CXX)

find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

add_executable(server server.c)
#add_executable(serverpp serverpp.cpp)
add_executable(client client.c)

target_link_libraries(server Threads::Threads)
