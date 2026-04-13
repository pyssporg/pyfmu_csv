cmake_minimum_required(VERSION 3.25)

project(pyfmu_csv VERSION 0.1.0 LANGUAGES CXX)

option(PYFMU_CSV_BUILD_TESTS "Build C++ test targets" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

add_subdirectory(runtime)
