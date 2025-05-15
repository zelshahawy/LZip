cmake_minimum_required(VERSION 3.10)
project(lzw_header_only LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_include_directories(lzw PUBLIC ${CMAKE_SOURCE_DIR}/include)
add_executable(lzw src/main.cpp)
