#pragma once

#include <algorithm>
#include <string>

#include "ssl_referee.pb.h"

#include "constants.h"

enum TeamCommand
{
  PREPARE_KICKOFF,
  PREPARE_PENALTY,
  DIRECT_FREE,
  INDIRECT_FREE,
  TIMEOUT,
  GOAL,
  BALL_PLACEMENT,
};

SSL_Referee::Command teamCommand(TeamCommand cmd, Team team);

Team commandTeam(SSL_Referee::Command command);

std::string commandDisplayName(SSL_Referee::Command command);
std::string stageDisplayName(SSL_Referee::Stage stage);

template <class num>
inline num sign(num x)
{
  return (x >= 0) ? (x > 0) : -1;
}

template <class num>
inline num sign_nz(num x)
{
  return (x >= 0) ? 1 : -1;
}

template <class num1, class num2>
inline num1 abs_bound(num1 x, num2 range)
// bound absolute value x in [-range,range]
{
  if (x < -range) {
    x = -range;
  }
  if (x > range) {
    x = range;
  }
  return x;
}

template <class num1, class num2>
inline num1 bound(num1 x, num2 low, num2 high)
{
  if (x < low) {
    x = low;
  }
  if (x > high) {
    x = high;
  }
  return x;
}

uint64_t GetTimeMicros();

Team RandomTeam();
