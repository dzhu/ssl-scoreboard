#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

#include "shared/util.h"

#include "autoref_comparer.h"

bool AutorefComparer::autoref_msg_equal(const ssl::SSL_Autoref &p1, const ssl::SSL_Autoref &p2)
{
  if (p1.event_case() != p2.event_case()) {
    return false;
  }

  switch (p1.event_case()) {
    case ssl::SSL_Autoref::kBallOutOfField: {
      return p1.ball_out_of_field().last_touch() == p2.ball_out_of_field().last_touch();
    }
    case ssl::SSL_Autoref::kFoul: {
      return p1.foul().foul_type() == p2.foul().foul_type() && p1.foul().offending_team() == p2.foul().offending_team();
    }
    case ssl::SSL_Autoref::kLackOfProgress: {
      return p1.lack_of_progress() == p2.lack_of_progress();
    }
    case ssl::SSL_Autoref::kGoal: {
      return p1.goal().scoring_team() == p2.goal().scoring_team();
    }
    case ssl::SSL_Autoref::EVENT_NOT_SET: {
      return true;
    }
  }
}

bool AutorefComparer::proc_msg(const ssl::SSL_Autoref &msg, const Address &src)
{
  printf("\nReceived message from %s.\n", src.toString().c_str());
  uint64_t now = GetTimeMicros();

  int n_found = 0;

  history.push_back(received_msg{now, msg, src});

  // Sources of matching messages.
  std::vector<Address> senders;

  // Find all senders that sent matching messages recently enough.
  for (auto iter = history.begin(); iter != history.end();) {
    received_msg old = *iter;

    // Clear and ignore messages that are too old.
    if (old.time < now - time_thresh_usec) {
      iter = history.erase(iter);
    }
    // Check if this message matches.
    else if (autoref_msg_equal(old.msg, msg) && find(senders.begin(), senders.end(), old.src) == senders.end()) {
      printf("Previous matching message from %s.\n", old.src.toString().c_str());

      senders.push_back(old.src);
      n_found++;
      iter++;
    }
    // If neither of the above, keep looking.
    else {
      iter++;
    }
  }

  return n_found >= n_autorefs;
}
