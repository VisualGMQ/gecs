cmake_minimum_required(VERSION 3.12)
project(gecs
    LANGUAGES CXX)

add_library(gecs INTERFACE)
target_include_directories(gecs INTERFACE "./src")
target_compile_features(gecs INTERFACE cxx_std_17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GECS_BUILD_TEST "enable unittest" OFF)

if (PROJECT_IS_TOP_LEVEL OR GECS_BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif()

option(GECS_BUILD_DEMO "build demo" OFF)

if (GECS_BUILD_DEMO)
    add_subdirectory(demo)
endif()