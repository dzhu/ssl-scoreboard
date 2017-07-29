#pragma once

#include <wx/wx.h>

#include "gui/control_frame.h"
#include "gui/event_types.h"
#include "gui/main_frame.h"

#include "shared/autoref_comparer.h"
#include "shared/tracker.h"
#include "shared/world.h"

#include "messages_robocup_ssl_wrapper.pb.h"
#include "ssl_autoref.pb.h"

#include "shared/rconclient.h"
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
  Tracker tracker;

  ScoreboardFrame *display_frame;
  ScoreboardControlFrame *control_frame;

  AutorefComparer comparer;

  RemoteClient rcon;

  void refresh()
  {
    display_frame->Refresh();
    control_frame->Refresh();
  }

  uint64_t replay_margin_usec;

public:
  World world;
  std::deque<World> world_history;

  SSL_DetectionFrame detection_msg;
  SSL_GeometryData geometry_msg;
  SSL_Referee referee_msg;

  uint64_t replay_start, replay_end, replay_actual_start;

  bool flip_sides;
  bool enable_replays, replays_follow_ball;

  bool enable_blue_placement, enable_yellow_placement;

  virtual bool OnInit();
  virtual int OnExit();

  ScoreboardApp(int argc, char **argv);

  void updateVision(const SSL_DetectionFrame &d);
  void updateGeometry(const SSL_GeometryData &g);
  void updateAutoref(const ssl::SSL_Autoref &a, const Address &src);
  void updateReferee(const SSL_Referee &r);
  void sendCommand(SSL_Referee::Command c);

  void setEnableReplays(bool e)
  {
    enable_replays = e;
  }
  void setFollowBall(bool e)
  {
    replays_follow_ball = e;
  }
  void setFlipSides(bool e)
  {
    flip_sides = e;
  }
  void setBluePlacement(bool v)
  {
    enable_blue_placement = v;
  }
  void setYellowPlacement(bool v)
  {
    enable_yellow_placement = v;
  }
};
