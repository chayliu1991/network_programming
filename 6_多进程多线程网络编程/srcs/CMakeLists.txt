cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)


add_executable(tcp_client tcp_client.c)
add_executable(tcp_multi_process_server tcp_multi_process_server.c)

add_executable(tcp_multi_thread_server tcp_multi_thread_server.c)
target_link_libraries(tcp_multi_thread_server -lpthread)

add_executable(tcp_thread_pool_server tcp_thread_pool_server.c)
target_link_libraries(tcp_thread_pool_server -lpthread)

