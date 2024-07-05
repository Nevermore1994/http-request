cmake_minimum_required(VERSION 3.20)
project(http-request)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_FILE_LISTS *.cpp)
set(PRIVATE_HEADER_FILE_LIST include/*.hpp include/*.h)
set(PUBLIC_HEADER_FILE_LIST include/public/*.hpp include/public/*.h)

find_package(Threads REQUIRED)


file(GLOB FILES ${SRC_FILE_LISTS} ${PRIVATE_HEADER_FILE_LIST} ${PUBLIC_HEADER_FILE_LIST})

add_library(${PROJECT_NAME} ${FILES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PUBLIC include/public)

option(DISABLE_HTTPS "disable https" OFF)

if (NOT DISABLE_HTTPS)
    message(STATUS "enable https")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_HTTPS)
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL OpenSSL::Crypto)
else()
    message(STATUS "disable https")
endif ()

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})