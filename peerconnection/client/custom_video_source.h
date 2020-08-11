#ifndef PEERCONNECTION_CLIENT_CUSTOM_VIDEO_SOURCE
#define PEERCONNECTION_CLIENT_CUSTOM_VIDEO_SOURCE

#include "api/video/video_frame.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_frame.h"
#include "api/test/frame_generator_interface.h"
#include "api/media_stream_interface.h"
#include "media/base/adapted_video_track_source.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_message.h"
#include "rtc_base/message_handler.h"
#include "rtc_base/location.h"

class CustomVideoSource : public rtc::AdaptedVideoTrackSource,
                          public rtc::Thread {
 public:
  explicit CustomVideoSource();
  ~CustomVideoSource() override;
  void InputVideoFrame();
  //rtc::AdaptedVideoTrackSource implementation
  webrtc::MediaSourceInterface::SourceState state() const override;
  bool remote() const override;
  bool is_screencast() const override;
  absl::optional<bool> needs_denoising() const override;

  //rtc::Thread implementation
  void Stop() override {
    running_ = false;
  }

  void Run() override {
    while (running_) {
      InputVideoFrame();
      rtc::Thread::SleepMs(1000);
    }
  }

  static rtc::scoped_refptr<CustomVideoSource> Create();
/*
  void Quit() override {
    return rtc::Thread::Quit();
  }

  bool IsQuitting() override {
    return rtc::Thread::IsQuitting();
  }

  void Restart() override {
    return rtc::Thread::Restart();
  }

  bool IsProcessingMessagesForTesting() override {
    return rtc::Thread::IsProcessingMessagesForTesting();
  }

  bool Get(rtc::Message* pmsg,
                int cmsWait = kForever,
                bool process_io = true) override {

    return rtc::Thread::Get(pmsg, cmsWait, process_io);
  }

  bool Peek(rtc::Message* pmsg, int cmsWait = 0) override {
    return rtc::Thread::Peek(pmsg, cmsWait);
  }

  void Post(const rtc::Location& posted_from,
                    rtc::MessageHandler* phandler,
                    uint32_t id = 0,
                    rtc::MessageData* pdata = nullptr,
                    bool time_sensitive = false) override {
      rtc::Thread::Post(posted_from, phandler, id, pdata, time_sensitive);
  }

  void PostDelayed(const rtc::Location& posted_from,
                           int delay_ms,
                           rtc::MessageHandler* phandler,
                           uint32_t id = 0,
                           rtc::MessageData* pdata = nullptr) override {
      rtc::Thread::PostDelayed(posted_from, delay_ms, phandler, id, pdata);
  }

  void PostAt(const rtc::Location& posted_from,
                      int64_t run_at_ms,
                      rtc::MessageHandler* phandler,
                      uint32_t id = 0,
                      rtc::MessageData* pdata = nullptr) override {
      rtc::Thread::PostDelayed(posted_from, run_at_ms, phandler, id, pdata);
  }

  void Clear(rtc::MessageHandler* phandler,
                     uint32_t id =rtc::MQID_ANY,
                     rtc::MessageList* removed = nullptr) override {
      rtc::Thread::Clear(phandler, id, removed);
  }

  void Dispatch(rtc::Message* pmsg) override {
    rtc::Thread::Dispatch(pmsg);
  }

  int GetDelay() override {
    return rtc::Thread::GetDelay();
  }

  void Send(const rtc::Location& posted_from,
                    rtc::MessageHandler* phandler,
                    uint32_t id = 0,
                    rtc::MessageData* pdata = nullptr) {
    rtc::Thread::Send(posted_from, phandler, id, pdata);
  }
*/
 private:
  int64_t next_timestamp_us_ = rtc::kNumMicrosecsPerMillisec;
  std::unique_ptr<webrtc::test::FrameGeneratorInterface> mpSquareGenerator;
  bool running_ = true;
  RTC_DISALLOW_COPY_AND_ASSIGN(CustomVideoSource);
};

#endif
