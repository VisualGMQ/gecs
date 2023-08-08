cmake_minimum_required(VERSION 3.12)
project(gecs
    LANGUAGES CXX)

add_library(gecs INTERFACE)
target_include_directories(gecs INTERFACE "./src")
target_compile_features(gecs INTERFACE cxx_std_17)

option(GECS_ENABLE_TEST "enable unittest" ON)
if (GECS_ENABLE_TEST)
    enable_testing()
    add_subdirectory(test)
endif()