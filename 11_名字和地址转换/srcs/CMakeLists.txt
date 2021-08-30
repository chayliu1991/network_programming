cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

add_executable(gethostbyname gethostbyname.c)
add_executable(getserv getserv.c)
add_executable(getaddrinfo getaddrinfo.c)
