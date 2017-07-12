#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>

#include <sys/select.h>

#include "ssl_autoref.pb.h"
#include "ssl_referee.pb.h"

#include "scoreboard.h"

#include "gui/event_types.h"
#include "shared/constants.h"
#include "shared/optionparser.h"
#include "shared/udp.h"

enum OptionIndex
{
  UNKNOWN,
  HELP,
  VERBOSE,
  EVAL,
  DRY,
};

const option::Descriptor options[] = {
  {UNKNOWN, 0, "", "", option::Arg::None, "A scoreboard for the SSL."},      //
  {HELP, 0, "h", "help", option::Arg::None, "-h, --help: print help"},       //
  {VERBOSE, 0, "v", "verbose", option::Arg::None, "-v,--verbose: verbose"},  //
  {0, 0, nullptr, nullptr, nullptr, nullptr},
};

wxThread::ExitCode NetworkRecvThread::Entry()
{
  UDP vision_net;
  if (!vision_net.open(VisionGroup, VisionPort, true)) {
    puts("SSL-Vision port open failed!");
  }

  UDP ref_net;
  if (!ref_net.open(RefGroup, RefPort, false)) {
    puts("Referee port open failed!");
  }

  UDP autoref_net;
  if (!autoref_net.open(AutorefGroup, AutorefPort, false)) {
    puts("Autoref port open failed!");
  }

  puts("Network initialized...");

  SSL_WrapperPacket vision_msg;
  SSL_Referee ref_msg;
  ssl::SSL_Autoref autoref_msg;

  fd_set read_fds;
  int n_fds = 1 + std::max(std::max(vision_net.getFd(), ref_net.getFd()), autoref_net.getFd());

  // Let the select time out after .1 seconds, so we can exit even if not
  // getting any packets. select on Linux modifies the struct, so we
  // re-initialize each time.
  struct timeval timeout;

  while (!TestDestroy()) {
    FD_ZERO(&read_fds);
    FD_SET(vision_net.getFd(), &read_fds);
    FD_SET(ref_net.getFd(), &read_fds);
    FD_SET(autoref_net.getFd(), &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000;

    select(n_fds, &read_fds, nullptr, nullptr, &timeout);

    if (FD_ISSET(vision_net.getFd(), &read_fds) && vision_net.recv(vision_msg)) {
      if (vision_msg.has_detection()) {
        board->CallAfter([=]() { board->updateVision(vision_msg.detection()); });
      }
      if (vision_msg.has_geometry()) {
        board->CallAfter([=]() { board->updateGeometry(vision_msg.geometry()); });
      }
    }
    if (FD_ISSET(ref_net.getFd(), &read_fds) && ref_net.recv(ref_msg)) {
      board->CallAfter([=]() { board->updateReferee(ref_msg); });
    }
    if (FD_ISSET(autoref_net.getFd(), &read_fds) && autoref_net.recv(autoref_msg)) {
      board->CallAfter([=]() { board->updateAutoref(autoref_msg); });
    }
  }

  return nullptr;
}

bool ScoreboardApp::OnInit()
{
  if (!wxApp::OnInit()) {
    return false;
  }

  argc -= (argc > 0);
  argv += (argc > 0);

  option::Stats stats(options, argc, argv);
  std::vector<option::Option> args(stats.options_max);
  std::vector<option::Option> buffer(stats.buffer_max);
  option::Parser parse(options, argc, argv, &args[0], &buffer[0]);

  if (parse.error() || args[HELP] != nullptr || args[UNKNOWN] != nullptr) {
    option::printUsage(std::cout, options);
    return false;
  }

  display_frame = new ScoreboardFrame(_T("SSL Scoreboard"), this);
  display_frame->Show(true);
  control_frame = new ScoreboardControlFrame(_T("Scoreboard control"), this);
  control_frame->Show(true);

  network_thread = new NetworkRecvThread(this);
  network_thread->Create();
  network_thread->Run();

  return true;
}

int ScoreboardApp::OnExit()
{
  network_thread->Delete();
  delete network_thread;

  return this->wxApp::OnExit();
}

void ScoreboardApp::updateVision(const SSL_DetectionFrame &d)
{
  tracker.updateVision(d);
  if (tracker.getWorld(world)) {
    world_history.push_back(world);
    // only store one minute of history
    if (world_history.size() > 60 * 60) {
      world_history.pop_front();
    }
    display_frame->Refresh();
  }
}

void ScoreboardApp::updateGeometry(const SSL_GeometryData &g)
{
  geometry_msg = g;
  display_frame->Refresh();
}

void ScoreboardApp::updateAutoref(const ssl::SSL_Autoref &a)
{
  display_frame->history_panel->update(a);
  display_frame->Refresh();
  if (enable_replays && a.has_replay()) {
    replay_start = a.replay().start_timestamp();
    replay_end = a.replay().end_timestamp();
    replay_actual_start = a.command_timestamp();
  }
}

void ScoreboardApp::updateReferee(const SSL_Referee &r)
{
  referee_msg = r;
  display_frame->info_panel->update();
  display_frame->GetSizer()->Layout();
  display_frame->Refresh();
}

int main(int argc, char *argv[])
{
  wxInitAllImageHandlers();

  wxApp::SetInstance(new ScoreboardApp(argc, argv));

  // also don't feel like wrangling wx
  int _argc = 0;
  char **_argv = {nullptr};
  wxEntryStart(_argc, _argv);

  wxTheApp->OnInit();
  wxTheApp->OnRun();
  wxTheApp->OnExit();
  wxEntryCleanup();
  return 0;
}
