cmake_minimum_required(VERSION 3.17)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

project(network LANGUAGES C CXX)

find_package(Threads REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_executable(server server.c)
add_executable(qtserver qtserver.cpp)
#add_executable(serverpp serverpp.cpp)
add_executable(client client.c)

target_link_libraries(server Threads::Threads)
target_link_libraries(qtserver PRIVATE Qt5::Core Qt5::Network)
