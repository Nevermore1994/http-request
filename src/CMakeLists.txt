cmake_minimum_required(VERSION 3.20)
project(http-request)

set(CMAKE_CXX_STANDARD 17)

set(SRC_FILE_LISTS *.cpp)
set(PRIVATE_HEADER_FILE_LIST include/*.hpp src/include/*.h)
set(PUBLIC_HEADER_FILE_LIST include/public/*.hpp include/public/*h)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB FILES ${SRC_FILE_LISTS} ${PRIVATE_HEADER_FILE_LIST} ${PUBLIC_HEADER_FILE_LIST})
add_library(${PROJECT_NAME} ${FILES})
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PUBLIC include/public)


target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

