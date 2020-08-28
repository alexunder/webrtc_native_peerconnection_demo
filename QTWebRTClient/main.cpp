#include "webrtcadapter.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDebug>

#include "flag_defs.h"
#include "conductor.h"

#include "absl/flags/parse.h"
#include "system_wrappers/include/field_trial.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/ssl_adapter.h"

#include "rtc_base/physical_socket_server.h"
#include "rtc_base/thread.h"

class CustomSocketServer : public rtc::PhysicalSocketServer {
 public:
  explicit CustomSocketServer(RTCMainWindow * wnd, QApplication * app)
      : wnd_(wnd), app_(app), conductor_(NULL), client_(NULL) {}
  virtual ~CustomSocketServer() {}

  void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  // Override so that we can also pump the GTK message loop.
  bool Wait(int cms, bool process_io) override {
    // Pump GTK events.
    // TODO(henrike): We really should move either the socket server or UI to a
    // different thread.  Alternatively we could look at merging the two loops
    // by implementing a dispatcher for the socket server and/or use
    // g_main_context_set_poll_func.
    //while (gtk_events_pending())
    //  gtk_main_iteration();

    app_->processEvents();

    if (!wnd_->IsWindow() && !conductor_->connection_active() &&
        client_ != NULL && !client_->is_connected()) {
      message_queue_->Quit();
    }
    return rtc::PhysicalSocketServer::Wait(0 /*cms == -1 ? 1 : cms*/,
                                           process_io);
  }

 protected:
  rtc::Thread* message_queue_;
  RTCMainWindow * wnd_;
  QApplication * app_;
  Conductor* conductor_;
  PeerConnectionClient* client_;
};


int main(int argc, char *argv[])
{
    const std::string forced_field_trials = absl::GetFlag(FLAGS_force_fieldtrials);
    webrtc::field_trial::InitFieldTrialsFromString(forced_field_trials.c_str());
    // Abort if the user specifies a port that is outside the allowed
    // range [1, 65535].
    if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
        printf("Error: %i is not a valid port.\n", absl::GetFlag(FLAGS_port));
        return -1;
    }

    const std::string server = absl::GetFlag(FLAGS_server);
    qDebug("Server: %s, port:%d\n", server.c_str(), absl::GetFlag(FLAGS_port));

    QApplication a(argc, argv);

    /*
    WebRTCAdapter * pWebrtcObject = WebRTCAdapter::getInstance();
    pWebrtcObject->setParameters(server.c_str(), absl::GetFlag(FLAGS_port),
                                     absl::GetFlag(FLAGS_autoconnect),
                                     absl::GetFlag(FLAGS_autocall));
    */

    RTCMainWindow w(nullptr, server.c_str(), absl::GetFlag(FLAGS_port),
                    absl::GetFlag(FLAGS_autoconnect),
                    absl::GetFlag(FLAGS_autocall));
    w.Create();

    CustomSocketServer socket_server(&w, &a);
    rtc::AutoSocketServerThread thread(&socket_server);

    rtc::InitializeSSL();
    PeerConnectionClient client;
    rtc::scoped_refptr<Conductor> conductor(
        new rtc::RefCountedObject<Conductor>(&client, &w));
    conductor->setCustomMode(true);

    thread.Run();

    w.Destroy();

    qDebug() << "Quit Main";
    rtc::CleanupSSL();
    return 0;
}
