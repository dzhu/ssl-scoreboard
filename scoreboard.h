#include <wx/wx.h>

#include "gui/control_frame.h"
#include "gui/event_types.h"
#include "gui/main_frame.h"

#include "messages_robocup_ssl_wrapper.pb.h"
#include "ssl_autoref.pb.h"

class ScoreboardApp;

class NetworkRecvThread : public wxThread
{
  ScoreboardApp *board;

protected:
  virtual wxThread::ExitCode Entry();

public:
  NetworkRecvThread(ScoreboardApp *board) : wxThread(wxTHREAD_JOINABLE), board(board)
  {
  }
};

class ScoreboardApp : public wxApp
{
  int argc;
  char **argv;
  NetworkRecvThread *network_thread;

  ScoreboardFrame *display_frame;
  //ScoreboardControlFrame *control_frame;

  void refresh()
  {
    display_frame->Refresh();
    // control_frame->Refresh();
  }

public:
  SSL_DetectionFrame detection_msg;
  SSL_GeometryData geometry_msg;
  // std::vector<ssl::SSL_Autoref> autoref_msgs;
  SSL_Referee referee_msg;

  virtual bool OnInit();
  virtual int OnExit();

  ScoreboardApp(int argc, char **argv) : argc(argc), argv(argv){};

  void updateVision(const SSL_DetectionFrame &d);
  void updateGeometry(const SSL_GeometryData &g);
  void updateAutoref(const ssl::SSL_Autoref &a);
  void updateReferee(const SSL_Referee &r);
};
