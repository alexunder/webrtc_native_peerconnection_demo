#ifndef WEBRTCADAPTER_H
#define WEBRTCADAPTER_H

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"

#include "absl/flags/parse.h"

#include "peer_connection_client.h"
#include "main_wnd.h"

class WebRTCAdapter : public MainWindow {
public:
    WebRTCAdapter();
    WebRTCAdapter(const char* server, int port, bool autoconnect, bool autocall);
    ~WebRTCAdapter();

    virtual void RegisterObserver(MainWndCallback* callback);
    virtual bool IsWindow();
    virtual void SwitchToConnectUI();
    virtual void SwitchToPeerList(const Peers& peers);
    virtual void SwitchToStreamingUI();
    virtual void MessageBox(const char* caption, const char* text, bool is_error);
    virtual MainWindow::UI current_ui();
    virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
    virtual void StopLocalRenderer();
    virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
    virtual void StopRemoteRenderer();
    virtual void QueueUIThreadCallback(int msg_id, void* data);
public:
    void ConnectServer();
    void ConnectPeer();
    bool isConnectedPeerMode();
    void ConsumeImage(unsigned char * image, size_t size);
    static WebRTCAdapter * getInstance();
    void setParameters(const char* server, int port, bool autoconnect,
                                      bool autocall);
private:
    static WebRTCAdapter *mpInstance;
protected:
    MainWndCallback* mCallback;
    std::string mServer;
    std::string mPort;
    bool mAutoConnect;
    bool mAutoCall;
    int mWidth;
    int mHeight;
    int mAnotherPeerID;
    bool mIsConnectedPeer;
};

#endif // WEBRTCADAPTER_H
