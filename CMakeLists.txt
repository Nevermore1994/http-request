cmake_minimum_required(VERSION 3.20)
project(example)
set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} example.cpp)
add_subdirectory(src)

option(DISABLE_TEST "disable tests" OFF)

if(NOT DISABLE_TEST)
    message(STATUS "enable testing")
    enable_testing()
    include(cmake/gtest.cmake)
    add_subdirectory(test)
else()
    message(STATUS "disable testing")
endif()

target_link_libraries(example http-request)
