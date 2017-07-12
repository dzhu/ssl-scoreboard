#include <cstdarg>

#include "util.h"

SSL_Referee::Command teamCommand(TeamCommand cmd, Team team)
{
  if (team != TeamBlue && team != TeamYellow) {
    team = RandomTeam();
  }

#define X(s) \
  case s:    \
    return (team == TeamBlue) ? SSL_Referee::s##_BLUE : SSL_Referee::s##_YELLOW

  switch (cmd) {
    X(PREPARE_KICKOFF);
    X(PREPARE_PENALTY);
    X(DIRECT_FREE);
    X(INDIRECT_FREE);
    X(TIMEOUT);
    X(GOAL);
    X(BALL_PLACEMENT);
    default:
      return SSL_Referee::HALT;
  }

#undef X
}

Team commandTeam(SSL_Referee::Command command)
{
  switch (command) {
    case SSL_Referee::TIMEOUT_BLUE:
    case SSL_Referee::GOAL_BLUE:
    case SSL_Referee::BALL_PLACEMENT_BLUE:
    case SSL_Referee::DIRECT_FREE_BLUE:
    case SSL_Referee::INDIRECT_FREE_BLUE:
    case SSL_Referee::PREPARE_KICKOFF_BLUE:
    case SSL_Referee::PREPARE_PENALTY_BLUE:
      return TeamBlue;

    case SSL_Referee::TIMEOUT_YELLOW:
    case SSL_Referee::GOAL_YELLOW:
    case SSL_Referee::BALL_PLACEMENT_YELLOW:
    case SSL_Referee::DIRECT_FREE_YELLOW:
    case SSL_Referee::INDIRECT_FREE_YELLOW:
    case SSL_Referee::PREPARE_KICKOFF_YELLOW:
    case SSL_Referee::PREPARE_PENALTY_YELLOW:
      return TeamYellow;

    case SSL_Referee::NORMAL_START:
    case SSL_Referee::STOP:
    case SSL_Referee::FORCE_START:
    case SSL_Referee::HALT:
      return TeamNone;
  }
}

std::string commandDisplayName(SSL_Referee::Command command)
{
  switch (command) {
    case SSL_Referee::TIMEOUT_BLUE:
      return "TIMEOUT BLUE";
    case SSL_Referee::GOAL_BLUE:
      return "GOAL BLUE";
    case SSL_Referee::BALL_PLACEMENT_BLUE:
      return "PLACEMENT BLUE";
    case SSL_Referee::DIRECT_FREE_BLUE:
      return "DIRECT BLUE";
    case SSL_Referee::INDIRECT_FREE_BLUE:
      return "INDIRECT BLUE";
    case SSL_Referee::PREPARE_KICKOFF_BLUE:
      return "KICKOFF BLUE";
    case SSL_Referee::PREPARE_PENALTY_BLUE:
      return "PENALTY BLUE";

    case SSL_Referee::TIMEOUT_YELLOW:
      return "TIMEOUT YELLOW";
    case SSL_Referee::GOAL_YELLOW:
      return "GOAL YELLOW";
    case SSL_Referee::BALL_PLACEMENT_YELLOW:
      return "PLACEMENT YELLOW";
    case SSL_Referee::DIRECT_FREE_YELLOW:
      return "DIRECT YELLOW";
    case SSL_Referee::INDIRECT_FREE_YELLOW:
      return "INDIRECT YELLOW";
    case SSL_Referee::PREPARE_KICKOFF_YELLOW:
      return "KICKOFF YELLOW";
    case SSL_Referee::PREPARE_PENALTY_YELLOW:
      return "PENALTY YELLOW";

    case SSL_Referee::NORMAL_START:
      return "START";
    case SSL_Referee::STOP:
      return "STOP";
    case SSL_Referee::FORCE_START:
      return "FORCE START";
    case SSL_Referee::HALT:
      return "HALT";
  }
}

std::string stageDisplayName(SSL_Referee::Stage stage)
{
  switch (stage) {
    case SSL_Referee::NORMAL_FIRST_HALF_PRE:
      return "PRE FIRST HALF";
    case SSL_Referee::NORMAL_FIRST_HALF:
      return "FIRST HALF";
    case SSL_Referee::NORMAL_HALF_TIME:
      return "HALF TIME";
    case SSL_Referee::NORMAL_SECOND_HALF_PRE:
      return "PRE SECOND HALF";
    case SSL_Referee::NORMAL_SECOND_HALF:
      return "SECOND HALF";
    case SSL_Referee::EXTRA_TIME_BREAK:
      return "EXTRA TIME BREAK";
    case SSL_Referee::EXTRA_FIRST_HALF_PRE:
      return "PRE EXTRA FIRST HALF";
    case SSL_Referee::EXTRA_FIRST_HALF:
      return "EXTRA FIRST HALF";
    case SSL_Referee::EXTRA_HALF_TIME:
      return "EXTRA HALF TIME";
    case SSL_Referee::EXTRA_SECOND_HALF_PRE:
      return "PRE EXTRA SECOND HALF";
    case SSL_Referee::EXTRA_SECOND_HALF:
      return "EXTRA SECOND HALF";
    case SSL_Referee::PENALTY_SHOOTOUT_BREAK:
      return "PENALTY SHOOTOUT BREAK";
    case SSL_Referee::PENALTY_SHOOTOUT:
      return "PENALTY SHOOTOUT";
    case SSL_Referee::POST_GAME:
      return "POST GAME";
  }
}

uint64_t GetTimeMicros()
{
  timespec tv;
  clock_gettime(CLOCK_REALTIME, &tv);
  return tv.tv_sec * 1000000 + tv.tv_nsec / 1000;
}

Team RandomTeam()
{
  static std::default_random_engine generator;
  static std::uniform_int_distribution<unsigned int> binary_dist;

  return binary_dist(generator) ? TeamYellow : TeamBlue;
}
