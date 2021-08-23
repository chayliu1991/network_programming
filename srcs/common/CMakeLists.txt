cmake_minimum_required(VERSION 3.1)

if (EPOLL_EXISTS)
    set(LIBSRCS
            log.c
            sock_ntop.c
            read.c
            tcp_server.c
            tcp_client.c
            event_loop.c
            channel.c
            acceptor.c
            channel_map.c
            poll_dispatcher.c
            thread_pool.c
            event_loop_thread.c
            utils.c
            epoll_dispatcher.c
            buffer.c
            tcp_connection.c
            http_server.c
            http_request.c
            http_response.c
            )
else ()
    set(LIBSRCS
            log.c
            sock_ntop.c
            read.c
            tcp_server.c
            tcp_client.c
            event_loop.c
            channel.c
            acceptor.c
            channel_map.c
            poll_dispatcher.c
            thread_pool.c
            event_loop_thread.c
            utils.c
            buffer.c
            tcp_connection.c
            http_server.c
            http_request.c
            http_response.c
            )
endif ()


add_library(common_lib STATIC ${LIBSRCS})
set_target_properties(common_lib PROPERTIES OUTPUT_NAME "common")

set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/libs)






