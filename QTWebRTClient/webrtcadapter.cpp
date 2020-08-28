#include "webrtcadapter.h"
#include <QDebug>
#include <QMessageBox>

WebRTCAdapter * WebRTCAdapter::mpInstance = nullptr;

WebRTCAdapter::WebRTCAdapter(const char* server,
                             int port,
                             bool autoconnect,
                             bool autocall)
    : mServer(server),
      mAutoConnect(autoconnect),
      mAutoCall(autocall),
      mAnotherPeerID(-1),
      mIsConnectedPeer(false) {
    mCallback = nullptr;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%i", port);
    mPort = buffer;
}

WebRTCAdapter::WebRTCAdapter()
    : mServer("localhost"),
      mAutoConnect(false),
      mAutoCall(false),
      mAnotherPeerID(-1),
      mIsConnectedPeer(false),
      mCallback(nullptr)
{
    mPort = "8888";
}

WebRTCAdapter::~WebRTCAdapter() {
    RTC_DCHECK(!IsWindow());
}

void WebRTCAdapter::setParameters(const char* server, int port, bool autoconnect,
                                  bool autocall) {
    mServer = server;
    mAutoConnect = autoconnect;
    mAutoCall = autocall;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%i", port);
    mPort = buffer;
}

void WebRTCAdapter::RegisterObserver(MainWndCallback* callback) {
    mCallback = callback;
}

bool WebRTCAdapter::IsWindow() {
  return false;
}

void WebRTCAdapter::MessageBox(const char* caption,
                            const char* text,
                            bool is_error) {
    if (is_error)
        QMessageBox::critical(nullptr, caption, text);
    else
        QMessageBox::warning(nullptr, caption, text);
}

MainWindow::UI WebRTCAdapter::current_ui() {
  return STREAMING;
}

void WebRTCAdapter::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::StopLocalRenderer() {
    RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::StartRemoteRenderer(
    webrtc::VideoTrackInterface* remote_video) {
    RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::StopRemoteRenderer() {
    RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::QueueUIThreadCallback(int msg_id, void* data) {
    RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::SwitchToStreamingUI() {
  RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::SwitchToPeerList(const Peers& peers) {
    RTC_LOG(INFO) << __FUNCTION__;
    for (Peers::const_iterator i = peers.begin(); i != peers.end(); ++i) {
        if ( std::strcmp(i->second .c_str(),"VR") != 0 ) {
            mAnotherPeerID = i->first;
            break;
        }
    }
    mAnotherPeerID = -1;
}

void WebRTCAdapter::SwitchToConnectUI() {
  RTC_LOG(INFO) << __FUNCTION__;
}

void WebRTCAdapter::ConnectServer() {
    int port = mPort.length() ? atoi(mPort.c_str()) : 0;
    mCallback->StartLogin(mServer, port);
}

void WebRTCAdapter::ConnectPeer() {
    if (mAnotherPeerID == -1) {
        qDebug() << "The mAnotherPeerID is void, just return.\n";
        return;
    }

    if (mCallback == nullptr) {
        qDebug() << "The mCallback is null, just return.";
        return;
    }
    mCallback->ConnectToPeer(mAnotherPeerID);
    mIsConnectedPeer = true;
}

bool WebRTCAdapter::isConnectedPeerMode() {
    return mIsConnectedPeer;
}

void WebRTCAdapter::ConsumeImage(unsigned char * image, size_t size) {
    if (mCallback == nullptr) {
        qDebug() << "The mCallback is null, just return.";
        return;
    }
    //mCallback->UpdateVideoFrame(image, size);
}

WebRTCAdapter * WebRTCAdapter::getInstance() {
    if (mpInstance == nullptr) {
        mpInstance = new WebRTCAdapter();
    }
    return mpInstance;
}
