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
  DRY,
  AUTOREFS,
};

static option::ArgStatus Numeric(const option::Option &option, bool msg)
{
  char *endptr = 0;
  if (option.arg != nullptr) {
    strtol(option.arg, &endptr, 10);
  }
  if (endptr != option.arg && *endptr == 0) {
    return option::ARG_OK;
  }
  return option::ARG_ILLEGAL;
}

const option::Descriptor options[] = {
  {UNKNOWN, 0, "", "", option::Arg::None, "A scoreboard for the SSL."},          //
  {HELP, 0, "h", "help", option::Arg::None, "-h, --help: print help"},           //
  {VERBOSE, 0, "v", "verbose", option::Arg::None, "-v,--verbose: verbose"},      //
  {AUTOREFS, 0, "a", "autorefs", Numeric, "-a,--autorefs: number of autorefs"},  //
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

  Address src;

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
    if (FD_ISSET(autoref_net.getFd(), &read_fds) && autoref_net.recv(autoref_msg, src)) {
      board->CallAfter([=]() { board->updateAutoref(autoref_msg, src); });
    }
  }

  return nullptr;
}

ScoreboardApp::ScoreboardApp(int argc, char **argv)
    : argc(argc),
      argv(argv),
      replay_margin_usec(250000),
      replay_start(0),
      replay_end(0),
      replay_actual_start(0),
      flip_sides(false),
      enable_replays(true),
      replays_follow_ball(true),
      enable_blue_placement(true),
      enable_yellow_placement(true)
{
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

  if (args[AUTOREFS]) {
    printf("autorefs: %lu\n", atol(args[AUTOREFS].arg));
    comparer.setNumAutorefs(atol(args[AUTOREFS].arg));
  }
  comparer.setTimeThresh(1000 * 1000);

  control_frame = new ScoreboardControlFrame(_T("Scoreboard control"), this);
  control_frame->Show(true);
  display_frame = new ScoreboardFrame(_T("SSL Scoreboard"), this);
  display_frame->Show(true);

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

void ScoreboardApp::updateAutoref(const ssl::SSL_Autoref &a, const Address &src)
{
  if (comparer.proc_msg(a, src)) {
    display_frame->history_panel->update(comparer.getMatchingMessages());
    display_frame->Refresh();

    if (enable_replays) {
      replay_start = 1e18;
      replay_end = 0;
      for (const auto &a : comparer.getMatchingMessages()) {
        if (a.has_replay()) {
          replay_start = std::min(replay_start, a.replay().start_timestamp());
          replay_end = std::max(replay_end, a.replay().end_timestamp());
        }
      }
      replay_start -= replay_margin_usec;
      replay_end += replay_margin_usec;
      replay_actual_start = a.command_timestamp();
    }

    if (!rcon.isOpened()) {
      rcon.open("localhost", 10007);
    }

    if (rcon.isOpened()) {
      // TODO actually decide on the right command
      rcon.sendCommand(SSL_Referee::STOP);

      if (comparer.hasDesignatedPoint() && (enable_blue_placement || enable_yellow_placement)) {
        Team placing_team = TeamNone;

        if (enable_blue_placement && !enable_yellow_placement) {
          placing_team = TeamBlue;
        }
        else if (!enable_blue_placement && enable_yellow_placement) {
          placing_team = TeamYellow;
        }
        else {
          auto flip_team
            = [](ssl::SSL_Autoref::Team team) { return team == ssl::SSL_Autoref::BLUE ? TeamYellow : TeamBlue; };

          switch (a.event_case()) {
            case ssl::SSL_Autoref::kBallOutOfField: {
              placing_team = flip_team(a.ball_out_of_field().last_touch());
              break;
            }
            case ssl::SSL_Autoref::kFoul: {
              placing_team = flip_team(a.foul().offending_team());
              break;
            }
            case ssl::SSL_Autoref::kLackOfProgress: {
              placing_team = RandomTeam();
              break;
            }
            case ssl::SSL_Autoref::kGoal: {
              placing_team = flip_team(a.goal().scoring_team());
              break;
            }
            case ssl::SSL_Autoref::EVENT_NOT_SET: {
              break;
            }
          }
        }

        if (placing_team != TeamNone) {
          rcon.sendCommand(
            placing_team == TeamBlue ? SSL_Referee::BALL_PLACEMENT_BLUE : SSL_Referee::BALL_PLACEMENT_YELLOW,
            comparer.getDesignatedPoint());
          printf("ball placement: <%.0f,%.0f>\n", comparer.getDesignatedPoint().x(), comparer.getDesignatedPoint().y());
        }
      }
    }
  }
}

void ScoreboardApp::sendCommand(SSL_Referee::Command c)
{
  if (!rcon.isOpened()) {
    rcon.open("localhost", 10007);
  }

  if (rcon.isOpened()) {
    rcon.sendCommand(c);
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
