#include "webrtcadapter.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QtWidgets/QListWidget>

RTCMainWindow::RTCMainWindow(QWidget *parent, const char* server,
                             int port,
                             bool autoconnect,
                             bool autocall) :
    QMainWindow(parent),
    ui(new Ui::RTCMainWindow),
    mServer(server),
    mAutoConnect(autoconnect),
    mAutoCall(autocall),
    mAnotherPeerID(-1),
    mIsConnectedPeer(false),
    mStatus(CONNECT_TO_SERVER)
{
    ui->setupUi(this);
    mCallback = nullptr;
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%i", port);
    mPort = buffer;
}

RTCMainWindow::~RTCMainWindow()
{
    delete ui;
}

void RTCMainWindow::HandleUIThreadCallback(int msg_id, void* data) {
    mCallback->UIThreadCallback(msg_id, data);
}

void RTCMainWindow::RegisterObserver(MainWndCallback* callback) {
    mCallback = callback;
}

bool RTCMainWindow::IsWindow() {
  return isWindow();
}

void RTCMainWindow::MessageBox(const char* caption,
                            const char* text,
                            bool is_error) {
    if (is_error)
        QMessageBox::critical(nullptr, caption, text);
    else
        QMessageBox::warning(nullptr, caption, text);
}

MainWindow::UI RTCMainWindow::current_ui() {
    qDebug() << __FUNCTION__;
    return mStatus;
}

void RTCMainWindow::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
}

void RTCMainWindow::StopLocalRenderer() {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
}

void RTCMainWindow::StartRemoteRenderer(
    webrtc::VideoTrackInterface* remote_video) {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
}

void RTCMainWindow::StopRemoteRenderer() {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
}

void RTCMainWindow::QueueUIThreadCallback(int msg_id, void* data) {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
    QMetaObject::invokeMethod(this, "HandleUIThreadCallback",
                              Qt::QueuedConnection, Q_ARG(int, msg_id), Q_ARG(void*, data));
}

void RTCMainWindow::SwitchToStreamingUI() {
  RTC_LOG(INFO) << __FUNCTION__;
  qDebug() << __FUNCTION__;
}

void RTCMainWindow::SwitchToPeerList(const Peers& peers) {
    RTC_LOG(INFO) << __FUNCTION__;
    qDebug() << __FUNCTION__;
    ui->peerListWidget->setVisible(true);

    for (Peers::const_iterator it = peers.begin(); it != peers.end(); ++it) {
        std::string item = it->second + " / " + std::to_string(it->first);
        ui->peerListWidget->addItem(item.c_str());
    }

    connect(ui->peerListWidget, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(OnPeerActivated(QListWidgetItem*)));
    ui->peerListWidget->show();

    mStatus = LIST_PEERS;
    /*
    for (Peers::const_iterator i = peers.begin(); i != peers.end(); ++i) {
        if ( std::strcmp(i->second .c_str(),"VR") != 0 ) {
            mAnotherPeerID = i->first;
            break;
        }
    }
    mAnotherPeerID = -1;
    */
}

void RTCMainWindow::OnPeerActivated(QListWidgetItem* peer) {
    const std::string& text = peer->text().toStdString();
    auto idx = text.rfind('/') + 2;
    int peer_num = atoi(text.c_str() + idx);
    mCallback->ConnectToPeer(peer_num);
}

void RTCMainWindow::closeEvent(QCloseEvent* event) {
    mCallback->Close();
}

bool RTCMainWindow::Create() {
    show();
    SwitchToConnectUI();
    return true;
}

bool RTCMainWindow::Destroy() {
    return true;
}

void RTCMainWindow::SwitchToConnectUI() {
  RTC_LOG(INFO) << __FUNCTION__;
  qDebug() << __FUNCTION__;
  ui->peerListWidget->setVisible(false);
  ui->ServerEdit->setText(QString::fromStdString(mServer));
  ui->PortEdit->setText(QString::fromStdString(mPort));
  mStatus = CONNECT_TO_SERVER;
}

void RTCMainWindow::ConnectServer() {
    ui->pushButton->setEnabled(false);
    int port = mPort.length() ? atoi(mPort.c_str()) : 0;
    mCallback->StartLogin(mServer, port);
}

void RTCMainWindow::ConnectPeer() {
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

bool RTCMainWindow::isConnectedPeerMode() {
    return mIsConnectedPeer;
}

void RTCMainWindow::on_pushButton_clicked()
{
    qDebug()<< "connect Button clicked.";
    ConnectServer();
}
