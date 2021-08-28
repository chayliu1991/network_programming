cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(udp_server udp_server.c)
add_executable(udp_client udp_client.c)

add_executable(udp_connect_server udp_connect_server.c)
add_executable(udp_connect_client udp_connect_client.c)



