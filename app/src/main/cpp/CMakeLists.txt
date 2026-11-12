cmake_minimum_required(VERSION 3.22.1)
project(valleycity3d LANGUAGES CXX)

add_library(valleycity3d SHARED native_renderer.cpp)
target_compile_features(valleycity3d PRIVATE cxx_std_17)
target_compile_options(valleycity3d PRIVATE -Wall -Wextra)

find_library(android-log log)
find_library(opengles-v2 GLESv2)
target_link_libraries(valleycity3d PRIVATE ${android-log} ${opengles-v2})
