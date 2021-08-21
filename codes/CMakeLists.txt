

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



