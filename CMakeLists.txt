cmake_minimum_required(VERSION 3.17)
set(THREADS_PREFER_PTHREAD_FLAG ON)

project(QtServerTest LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(Qt5 COMPONENTS Core Network REQUIRED)

add_executable(client client.c)
add_executable(server server.c)
add_executable(server_qt server_qt.cpp)

target_link_libraries(server_qt PUBLIC Threads::Threads Qt5::Core Qt5::Network)
target_link_libraries(server PUBLIC Threads::Threads)
