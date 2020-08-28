#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "peer_connection_client.h"
#include <QMainWindow>


#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"

#include "absl/flags/parse.h"
#include "main_wnd.h"

class QListWidget;
class QListWidgetItem;

namespace Ui {
class RTCMainWindow;
}

class RTCMainWindow : public QMainWindow, public MainWindow
{
    Q_OBJECT

public:
    explicit RTCMainWindow(QWidget *parent, const char* server,
                           int port, bool autoconnect, bool autocall);
    ~RTCMainWindow();

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

    bool Create();
    bool Destroy();
public:
    void ConnectServer();
    void ConnectPeer();
    bool isConnectedPeerMode();

protected slots:
     void HandleUIThreadCallback(int msg_id, void* data);
     void OnPeerActivated(QListWidgetItem* peer);
protected:
     virtual void closeEvent(QCloseEvent* event);
private slots:
    void on_pushButton_clicked();
private:
    MainWndCallback* mCallback;
    std::string mServer;
    std::string mPort;
    bool mAutoConnect;
    bool mAutoCall;
    int mWidth;
    int mHeight;
    int mAnotherPeerID;
    bool mIsConnectedPeer;
private:
    Ui::RTCMainWindow *ui;
    MainWindow::UI mStatus;
};

#endif // MAINWINDOW_H
