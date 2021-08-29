cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(poll_sercer poll_sercer.c)
add_executable(select_server select_server.c)
add_executable(epoll_server epoll_server.c)

add_executable(epoll_server_edge_trigger epoll_server_edge_trigger.c)
add_executable(epoll_server_level_trigger epoll_server_level_trigger.c)

