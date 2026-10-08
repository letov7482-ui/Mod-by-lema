cmake_minimum_required(VERSION 3.18.1)
project("modmenu")

add_library(modmenu SHARED
    native-lib.cpp
    bypass.cpp
    lua_inject.cpp)

find_library(log-lib log)
find_library(android-lib android)

target_link_libraries(modmenu
    ${log-lib}
    ${android-lib}
    dobby)

include_directories(${CMAKE_SOURCE_DIR}/dobby)
add_subdirectory(${CMAKE_SOURCE_DIR}/dobby ${CMAKE_BINARY_DIR}/dobby)
