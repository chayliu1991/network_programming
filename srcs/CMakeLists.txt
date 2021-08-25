cmake_minimum_required(VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

add_executable(make_socket make_socket.c)

add_executable(tcp_server tcp_server.c)
add_executable(tcp_client tcp_client.c)

add_executable(udp_server udp_server.c)
add_executable(udp_client udp_client.c)

add_executable(local_tcp_server local_tcp_server.c)
add_executable(local_tcp_client local_tcp_client.c)

add_executable(local_udp_server local_udp_server.c)
add_executable(local_udp_client local_udp_client.c)

add_executable(grace_server grace_server.c)
add_executable(grace_client grace_client.c)

add_executable(ping_server ping_server.c)
add_executable(ping_client ping_client.c)

add_executable(batch_write batch_write.c)

add_executable(udp_connect_client udp_connect_client.c)
add_executable(udp_connect_cli udp_connect_cli.c)
add_executable(udp_connect_srv udp_connect_srv.c)

add_executable(addr_used addr_used.c)
add_executable(addr_reuse addr_reuse.c)

add_executable(stream_client stream_client.c)
add_executable(stream_server stream_server.c)

add_executable(reliable_client reliable_client.c)
add_executable(reliable_server reliable_server.c)

add_executable(reliable_cli reliable_cli.c)
add_executable(reliable_sev reliable_sev.c)

add_executable(select_client select_client.c)

add_executable(poll_server poll_server.c)

add_executable(nonblocking_client nonblocking_client.c)
add_executable(nonblocking_server nonblocking_server.c)

add_executable(epoll_server epoll_server.c)
add_executable(epoll_server_edge_trigger epoll_server_edge_trigger.c)
add_executable(epoll_server_level_trigger epoll_server_level_trigger.c)

add_executable(multi_process_server multi_process_server.c)

add_executable(multi_thread_server multi_thread_server.c)
target_link_libraries(multi_thread_server -lpthread)

add_executable(thread_pool_server thread_pool_server.c)
target_link_libraries(thread_pool_server -lpthread)

add_executable(aio aio.c)
target_link_libraries(aio -lrt)

add_subdirectory(common)
link_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${PROJECT_SOURCE_DIR}/common)

add_executable(reactor_single_thread_server reactor_single_thread_server.c)
target_link_libraries(reactor_single_thread_server libcommon.a -lpthread)

add_executable(reactor_multi_thread_server reactor_multi_thread_server.c)
target_link_libraries(reactor_multi_thread_server libcommon.a -lpthread)

add_executable(http_server http_server.c)
target_link_libraries(http_server libcommon.a -lpthread)

