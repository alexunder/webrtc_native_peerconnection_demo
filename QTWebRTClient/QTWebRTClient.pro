#-------------------------------------------------
#
# Project created by QtCreator 2020-08-27T10:13:32
#
#-------------------------------------------------

QT       += quick core gui

CONFIG += c++14
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QTWebRTClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        webrtcadapter.cpp \
        ../peerconnection/client/peer_connection_client.cc \
        ../peerconnection/client/conductor.cc \
        ../peerconnection/client/defaults.cc \
        ../peerconnection/client/custom_video_source.cc


HEADERS += \
        mainwindow.h \
        webrtcadapter.h

#Adding webrtc include and libraries
#include section
INCLUDEPATH += $$PWD/../webrtc/include
INCLUDEPATH += $$PWD/../third_party
INCLUDEPATH += $$PWD/../third_party/libyuv/include
INCLUDEPATH += $$PWD/..
INCLUDEPATH += $$PWD/../peerconnection/client
#Cflags
QMAKE_CFLAGS += -Wno-deprecated-declarations -fno-strict-aliasing --param=ssp-buffer-size=4
QMAKE_CFLAGS += -fstack-protector -funwind-tables -fPIC -pipe -pthread -m64 -march=x86-64
QMAKE_CFLAGS += -Wno-builtin-macro-redefined -fexceptions
QMAKE_CFLAGS += -Wall -Wno-unused-local-typedefs -Wno-maybe-uninitialized
QMAKE_CFLAGS += -Wno-deprecated-declarations -Wno-comments -Wno-packed-not-aligned
QMAKE_CFLAGS += -Wno-missing-field-initializers -Wno-unused-parameter -O2 -fno-ident
QMAKE_CFLAGS += --fdata-sections -ffunction-sections -fno-omit-frame-pointer
QMAKE_CFLAGS += -g0 -fvisibility=hidden

QMAKE_CXXFLAGS += -std=gnu++14 -Wno-narrowing -Wno-class-memaccess
QMAKE_CXXFLAGS += -fvisibility-inlines-hidden -Wnon-virtual-dtor -Woverloaded-virtual
QMAKE_CXXFLAGS += -W -Wall  -Wno-deprecated-declarations -Wno-unused-parameter

DEFINES += WEBRTC_LINUX=1 WEBRTC_POSIX=1 HAVE_SYS_TIME_H

QMAKE_LFLAGS += -Wl,--fatal-warnings
QMAKE_LFLAGS += -Wl,--build-id -fPIC
QMAKE_LFLAGS += -Wl,-z,noexecstack
QMAKE_LFLAGS += -Wl,-z,relro
QMAKE_LFLAGS += -Wl,-z,now
QMAKE_LFLAGS += -Wl,-z,defs
QMAKE_LFLAGS += -Wl,--as-needed -fuse-ld=gold
QMAKE_LFLAGS += -Wl,--threads
QMAKE_LFLAGS += -Wl,--thread-count=4 -m64
QMAKE_LFLAGS += -Wl,-O2
QMAKE_LFLAGS += -Wl,--gc-sections -rdynamic -pie
QMAKE_LFLAGS += -Wl,--disable-new-dtags
QMAKE_LFLAGS += -lX11 -lXcomposite -lXext -lXrender -latomic -ldl -lpthread -lrt -lm -lz

LIBS += ../webrtc/obj/api/create_frame_generator/create_frame_generator.o
LIBS += ../webrtc/obj/test/field_trial/field_trial.o
LIBS += ../webrtc/obj/test/platform_video_capturer/platform_video_capturer.o
LIBS += ../webrtc/obj/test/platform_video_capturer/vcm_capturer.o
LIBS += ../webrtc/obj/test/rtp_test_utils/rtcp_packet_parser.o
LIBS += ../webrtc/obj/test/rtp_test_utils/rtp_file_reader.o
LIBS += ../webrtc/obj/test/rtp_test_utils/rtp_file_writer.o
LIBS += ../webrtc/obj/test/rtp_test_utils/rtp_header_parser.o
LIBS += ../webrtc/obj/test/video_test_common/fake_texture_frame.o
LIBS += ../webrtc/obj/test/video_test_common/frame_forwarder.o
LIBS += ../webrtc/obj/test/video_test_common/frame_generator_capturer.o
LIBS += ../webrtc/obj/test/video_test_common/test_video_capturer.o
LIBS += ../webrtc/obj/test/frame_generator_impl/frame_generator.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/flag/flag.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/parse/parse.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/ascii.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/escaping.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv_bigint.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/charconv_parse.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/memutil.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/match.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/numbers.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_cat.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_replace.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/str_split.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/string_view.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/strings/substitute.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/internal/escaping.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/internal/ostringstream.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/internal/utf8.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/raw_logging_internal/raw_logging.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/log_severity/log_severity.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/base/cycleclock.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/base/spinlock.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/base/sysinfo.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/base/thread_identity.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/base/unscaledcycleclock.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/dynamic_annotations/dynamic_annotations.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/spinlock_wait/spinlock_wait.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/throw_delegate/throw_delegate.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/numeric/int128/int128.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/types/bad_optional_access/bad_optional_access.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/types/bad_variant_access/bad_variant_access.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/config/usage_config.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/program_name/program_name.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/barrier.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/blocking_counter.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/create_thread_identity.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/per_thread_sem.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/waiter.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/mutex.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/synchronization/notification.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/synchronization/graphcycles_internal/graphcycles.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/base/malloc_internal/low_level_alloc.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/time/civil_time.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/time/clock.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/time/duration.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/time/format.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/time/time.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/civil_time/civil_time_detail.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_fixed.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_format.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_if.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_impl.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_info.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_libc.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_lookup.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/time_zone_posix.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/time/internal/cctz/time_zone/zone_info_source.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/stacktrace/stacktrace.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/address_is_readable.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/elf_mem_image.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/debugging_internal/vdso_support.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/symbolize/symbolize.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/debugging/demangle_internal/demangle.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/flag_internal/flag.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/commandlineflag/commandlineflag.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/commandlineflag_internal/commandlineflag.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/marshalling/marshalling.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/arg.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/bind.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/extension.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/float_conversion.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/output.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/strings/str_format_internal/parser.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/reflection/reflection.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/private_handle_accessor/private_handle_accessor.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/usage/usage.o
LIBS += ../webrtc/obj/third_party/abseil-cpp/absl/flags/usage_internal/usage.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/chacha-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/aes128gcmsiv-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/chacha20_poly1305_x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/aesni-gcm-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/aesni-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/ghash-ssse3-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/ghash-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/md5-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/p256-x86_64-asm.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/p256_beeu-x86_64-asm.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/rdrand-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/rsaz-avx2.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/sha1-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/sha256-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/sha512-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/vpaes-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/x86_64-mont.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/x86_64-mont5.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/trampoline-x86_64.o
LIBS += ../webrtc/obj/third_party/boringssl/boringssl_asm/poly_rq_mul.o
LIBS += ../webrtc/obj/third_party/jsoncpp/jsoncpp/json_reader.o
LIBS += ../webrtc/obj/third_party/jsoncpp/jsoncpp/json_value.o
LIBS += ../webrtc/obj/third_party/jsoncpp/jsoncpp/json_writer.o

LIBS += ../webrtc/obj/libwebrtc.a
LIBS += ../webrtc/obj/api/libaudio_options_api.a
LIBS += ../webrtc/obj/api/libcreate_peerconnection_factory.a
LIBS += ../webrtc/obj/api/libjingle_peerconnection_api.a
LIBS += ../webrtc/obj/api/libmedia_stream_interface.a
LIBS += ../webrtc/obj/api/audio_codecs/libaudio_codecs_api.a

LIBS += ../webrtc/obj/api/video/libvideo_frame_i420.a
LIBS += ../webrtc/obj/api/video/libvideo_rtp_headers.a
LIBS += ../webrtc/obj/api/video_codecs/libvideo_codecs_api.a
LIBS += ../webrtc/obj/rtc_base/libchecks.a
LIBS += ../webrtc/obj/rtc_base/librtc_base.a
LIBS += ../webrtc/obj/rtc_base/librtc_base_approved.a
LIBS += ../webrtc/obj/rtc_base/librtc_json.a
LIBS += ../webrtc/obj/rtc_base/libstringutils.a
LIBS += ../webrtc/obj/rtc_base/libplatform_thread_types.a
LIBS += ../webrtc/obj/rtc_base/liblogging.a
LIBS += ../webrtc/obj/rtc_base/libtimeutils.a
LIBS += ../webrtc/obj/rtc_base/libplatform_thread.a
LIBS += ../webrtc/obj/rtc_base/librtc_event.a
LIBS += ../webrtc/obj/rtc_base/librtc_task_queue.a
LIBS += ../webrtc/obj/rtc_base/libcriticalsection.a
LIBS += ../webrtc/obj/rtc_base/librtc_numerics.a
LIBS += ../webrtc/obj/rtc_base/librtc_task_queue_libevent.a
LIBS += ../webrtc/obj/rtc_base/libaudio_format_to_string.a
LIBS += ../webrtc/obj/rtc_base/librate_limiter.a
LIBS += ../webrtc/obj/rtc_base/libweak_ptr.a

FORMS += \
        mainwindow.ui
