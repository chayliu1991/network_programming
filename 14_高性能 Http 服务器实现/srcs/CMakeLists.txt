cmake_minimum_required (VERSION 3.1)
set(CMAKE_VERBOSE_MAKEFILE ON)


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/bin)

add_subdirectory(common)

include_directories(./common)
link_directories(./build/libs)

add_executable(reactor_single_thread_server reactor_single_thread_server.c)
target_link_libraries(reactor_single_thread_server libcommon.a -lpthread)

add_executable(reactor_multi_thread_server reactor_multi_thread_server.c)
target_link_libraries(reactor_multi_thread_server libcommon.a -lpthread)

add_executable(http_server http_server.c)
target_link_libraries(http_server libcommon.a -lpthread)
