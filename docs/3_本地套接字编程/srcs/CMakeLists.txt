cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(local_udp_client local_udp_client.c)
add_executable(local_udp_server local_udp_server.c)

# add_executable(local_tcp_client local_tcp_client.c)
# add_executable(local_tcp_server local_tcp_server.c)
