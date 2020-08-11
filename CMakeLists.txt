cmake_minimum_required(VERSION 3.5)
project(webrtc-native)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


#message(STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("webrtc/include")
include_directories("third_party")
include_directories("third_party/libyuv/include")

link_directories("webrtc/obj")
link_directories("webrtc/obj/api")
link_directories("webrtc/obj/api/audio_codecs")
link_directories("webrtc/obj/api/video")
link_directories("webrtc/obj/api/video_codecs")
link_directories("webrtc/obj/rtc_base")

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
list(APPEND FC_DEP_LIBS ${GTK3_LIBRARIES})
add_definitions(${GTK3_CFLAGS_OTHER})


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -fno-strict-aliasing --param=ssp-buffer-size=4")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector -funwind-tables -fPIC -pipe -pthread -m64 -march=x86-64")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-builtin-macro-redefined -D__DATE__= -D__TIME__= -D__TIMESTAMP__=")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wno-unused-local-typedefs -Wno-maybe-uninitialized")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations -Wno-comments -Wno-packed-not-aligned")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-missing-field-initializers -Wno-unused-parameter -O2 -fno-ident")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --fdata-sections -ffunction-sections -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g0 -fvisibility=hidden")




set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wno-narrowing -Wno-class-memaccess -fno-exceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility-inlines-hidden -Wnon-virtual-dtor -Woverloaded-virtual")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall  -Wno-deprecated-declarations -Wno-unused-parameter")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWEBRTC_LINUX=1 -DWEBRTC_POSIX=1 -DHAVE_SYS_TIME_H")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--fatal-warnings")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--build-id -fPIC")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,noexecstack")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,relro")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,now")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-z,defs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -fuse-ld=gold")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--threads")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--thread-count=4 -m64")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-O2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections -rdynamic -pie")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--disable-new-dtags")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lX11 -lXcomposite -lXext -lXrender -latomic -ldl -lpthread -lrt -lm -lz")

set(SRC
    peerconnection/client/linux/main.cc
    peerconnection/client/linux/main_wnd.cc
    peerconnection/client/peer_connection_client.cc
    peerconnection/client/conductor.cc
    peerconnection/client/defaults.cc
    peerconnection/client/custom_video_source.cc
    )

SET(OBJS
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/api/create_frame_generator/create_frame_generator.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/field_trial/field_trial.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/platform_video_capturer/platform_video_capturer.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/platform_video_capturer/vcm_capturer.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/rtp_test_utils/rtcp_packet_parser.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/rtp_test_utils/rtp_file_reader.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/rtp_test_utils/rtp_file_writer.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/rtp_test_utils/rtp_header_parser.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/video_test_common/fake_texture_frame.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/video_test_common/frame_forwarder.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/video_test_common/frame_generator_capturer.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/video_test_common/test_video_capturer.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/test/frame_generator_impl/frame_generator.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/flag/flag.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/parse/parse.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/ascii.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/escaping.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv_bigint.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv_parse.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/memutil.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/match.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/numbers.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_cat.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_replace.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_split.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/string_view.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/strings/substitute.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/internal/escaping.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/internal/ostringstream.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/internal/utf8.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/raw_logging_internal/raw_logging.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/log_severity/log_severity.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/base/cycleclock.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/base/spinlock.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/base/sysinfo.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/base/thread_identity.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/base/unscaledcycleclock.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/dynamic_annotations/dynamic_annotations.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/spinlock_wait/spinlock_wait.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/throw_delegate/throw_delegate.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/numeric/int128/int128.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/types/bad_optional_access/bad_optional_access.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/types/bad_variant_access/bad_variant_access.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/config/usage_config.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/program_name/program_name.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/barrier.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/blocking_counter.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/create_thread_identity.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/per_thread_sem.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/waiter.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/mutex.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/notification.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/synchronization/graphcycles_internal/graphcycles.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/base/malloc_internal/low_level_alloc.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/time/civil_time.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/time/clock.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/time/duration.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/time/format.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/time/time.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/civil_time/civil_time_detail.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_fixed.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_format.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_if.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_impl.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_info.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_libc.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_lookup.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_posix.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/zone_info_source.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/stacktrace/stacktrace.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/address_is_readable.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/elf_mem_image.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/vdso_support.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/symbolize/symbolize.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/debugging/demangle_internal/demangle.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/flag_internal/flag.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/commandlineflag/commandlineflag.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/commandlineflag_internal/commandlineflag.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/marshalling/marshalling.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/arg.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/bind.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/extension.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/float_conversion.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/output.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/parser.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/reflection/reflection.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/private_handle_accessor/private_handle_accessor.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/usage/usage.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/abseil-cpp/absl/flags/usage_internal/usage.o

    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/chacha-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/aes128gcmsiv-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/chacha20_poly1305_x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/aesni-gcm-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/aesni-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/ghash-ssse3-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/ghash-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/md5-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/p256-x86_64-asm.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/p256_beeu-x86_64-asm.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/rdrand-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/rsaz-avx2.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/sha1-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/sha256-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/sha512-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/vpaes-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/x86_64-mont.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/x86_64-mont5.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/trampoline-x86_64.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/boringssl/boringssl_asm/poly_rq_mul.o

    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/jsoncpp/jsoncpp/json_reader.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/jsoncpp/jsoncpp/json_value.o
    ${CMAKE_CURRENT_SOURCE_DIR}/webrtc/obj/third_party/jsoncpp/jsoncpp/json_writer.o
)

SET_SOURCE_FILES_PROPERTIES(
  ${OBJS}
  PROPERTIES
  EXTERNAL_OBJECT true
  GENERATED true
)

add_executable(webrtc_client ${OBJS} ${SRC})

target_link_libraries(webrtc_client
-Wl,--start-group
libwebrtc.a
libaudio_options_api.a
libcreate_peerconnection_factory.a
libjingle_peerconnection_api.a
libmedia_stream_interface.a
libaudio_codecs_api.a
libvideo_frame_i420.a
libvideo_rtp_headers.a
libvideo_codecs_api.a
libchecks.a
librtc_base.a
librtc_base_approved.a
librtc_json.a
libstringutils.a
libplatform_thread_types.a
liblogging.a
libtimeutils.a
libplatform_thread.a
librtc_event.a
librtc_task_queue.a
libcriticalsection.a
librtc_numerics.a
librtc_task_queue_libevent.a
libaudio_format_to_string.a
librate_limiter.a
libweak_ptr.a
${GTK3_LIBRARIES}
-Wl,--end-group
)

set(SSRC
    peerconnection/server/data_socket.cc
    peerconnection/server/main.cc
    peerconnection/server/peer_channel.cc
    peerconnection/server/utils.cc
)

add_executable(webrtc_server ${OBJS} ${SSRC})

target_link_libraries(webrtc_server
libwebrtc.a
)
