cmake_minimum_required(VERSION 3.1)
project(webrtc-native)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_C_FLAGS "-pthread")

include_directories("${LIBWEBRTC_PATH}/include")
include_directories("${LIBWEBRTC_PATH}/include/third_party/abseil-cpp")
include_directories("${LIBWEBRTC_PATH}/include/third_party/libyuv")
include_directories("${LIBWEBRTC_PATH}/include/third_party")
include_directories(".")

link_directories("${LIBWEBRTC_PATH}/lib")
list(APPEND webrtc_libs "webrtc")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
list(APPEND FC_DEP_LIBS ${GTK3_LIBRARIES})
add_definitions(${GTK3_CFLAGS_OTHER})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -pthread")

set(SRC
    client/linux/main.cc
    client/linux/main_wnd.cc
    client/peer_connection_client.cc
    client/conductor.cc
    client/defaults.cc
    test/vcm_capturer.cc
    test/test_video_capturer.cc
    )

add_executable(webrtc_client ${SRC})

target_link_libraries(webrtc_client ${webrtc_libs})
target_link_libraries(webrtc_client ${GTK3_LIBRARIES})
target_link_libraries(webrtc_client libjsoncpp.a)
target_link_libraries(webrtc_client libjpeg.a)

set(SSRC
    server/data_socket.cc
    server/main.cc
    server/peer_channel.cc
    server/utils.cc
    )

add_executable(webrtc_server ${SSRC})

target_link_libraries(webrtc_server ${webrtc_libs})
