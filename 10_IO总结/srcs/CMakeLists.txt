cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

add_executable(aio aio.c)
target_link_libraries(aio -lrt)