
#include "custom_video_source.h"
#include "api/test/create_frame_generator.h"
#include "api/video/video_frame.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/socket_server.h"

CustomVideoSource::CustomVideoSource()
    : Thread(rtc::SocketServer::CreateDefault(), /*do_init=*/false) {
    if (!rtc::ThreadManager::Instance()->CurrentThread()) {
      DoInit();
      rtc::ThreadManager::Instance()->SetCurrentThread(this);
    }

    int width  = 1920;
    int height = 2160;
    mpSquareGenerator = webrtc::test::CreateSquareFrameGenerator(width, height,
            webrtc::test::FrameGeneratorInterface::OutputType::kI420,
             absl::optional<int>(300));
}

CustomVideoSource::~CustomVideoSource() {
    Stop();
}

rtc::scoped_refptr<CustomVideoSource> CustomVideoSource::Create() {
    return rtc::scoped_refptr<CustomVideoSource>(new
        rtc::RefCountedObject<CustomVideoSource>());
}

void CustomVideoSource::InputVideoFrame() {
    webrtc::test::FrameGeneratorInterface::VideoFrameData frame_data =
        mpSquareGenerator->NextFrame();
    webrtc::VideoFrame input_frame = webrtc::VideoFrame::Builder()
                               .set_video_frame_buffer(frame_data.buffer)
                               .set_update_rect(frame_data.update_rect)
                               .build();
    OnFrame(input_frame);
}

bool CustomVideoSource::is_screencast() const
{
	return true;
}

absl::optional<bool> CustomVideoSource::needs_denoising() const
{
	return false;
}
webrtc::MediaSourceInterface::SourceState CustomVideoSource::state() const
{
	return webrtc::MediaSourceInterface::kLive;
}
bool CustomVideoSource::remote() const
{
	return false;
}
