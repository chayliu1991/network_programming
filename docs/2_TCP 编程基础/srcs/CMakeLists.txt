cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(network_info network_info.c)
add_executable(tcp_client tcp_client.c)
add_executable(tcp_server tcp_server.c)