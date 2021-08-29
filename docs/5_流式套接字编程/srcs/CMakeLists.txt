cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(tcp_client tcp_client.c)
add_executable(tcp_server tcp_server.c)

add_executable(ping_pang_server ping_pang_server.c)
add_executable(ping_pang_client ping_pang_client.c)

add_executable(batch_write batch_write.c)
