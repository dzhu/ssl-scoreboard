#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

#include "shared/util.h"
#include "shared/vector.h"

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

void AutorefComparer::computeDesignatedPoint(std::vector<received_msg> msgs)
{
  has_designated_point = false;

  if (!msgs.size()) {
    return;
  }

  switch (msgs[0].msg.event_case()) {
    case ssl::SSL_Autoref::kBallOutOfField: {
      // should compare multiple sources, but I'm going to be lazy right now
      auto msg = msgs[0].msg;
      auto out_point = msg.ball_out_of_field().position();
      bool past_goal_line = fabs(out_point.x()) - fabs(out_point.y()) > FieldLengthH - FieldWidthH;
      if (past_goal_line) {
        bool own_half
          = (msg.ball_out_of_field().last_touch() == ssl::SSL_Autoref::BLUE) == (msg.blue_side() * out_point.x() > 0);

        designated_point.set_x((abs(out_point.x()) - (own_half ? 100 : 500)) * (out_point.x() > 0 ? 1 : -1));
      }
      else {
        designated_point.set_x(out_point.x());
      }
      designated_point.set_y(2900 * (out_point.y() > 0 ? 1 : -1));
      has_designated_point = true;
      break;
    }
    case ssl::SSL_Autoref::kFoul: {
      vector2f sum;
      int n = 0;

      for (const received_msg &msg : msgs) {
        if (msg.msg.foul().has_designated_point()) {
          printf("in pt: %.0f,%.0f\n", msg.msg.foul().designated_point().x(), msg.msg.foul().designated_point().y());
          sum.x += msg.msg.foul().designated_point().x();
          sum.y += msg.msg.foul().designated_point().y();
          n++;
        }
      }

      if (n > 0) {
        has_designated_point = true;
        designated_point.set_x(sum.x / n);
        designated_point.set_y(sum.y / n);
      }
      break;
    }
    case ssl::SSL_Autoref::kLackOfProgress: {
      break;
    }
    case ssl::SSL_Autoref::kGoal: {
      has_designated_point = true;
      designated_point.set_x(0);
      designated_point.set_y(0);
      break;
    }
    case ssl::SSL_Autoref::EVENT_NOT_SET: {
      break;
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

  std::vector<received_msg> matches;

  // Find all senders that sent matching messages recently enough.
  for (auto iter = history.begin(); iter != history.end();) {
    received_msg old = *iter;

    // Clear and ignore messages that are too old.
    if (old.time < now - time_thresh_usec) {
      iter = history.erase(iter);
    }
    // Check if this message matches.
    else if (autoref_msg_equal(old.msg, msg) && find(senders.begin(), senders.end(), old.src) == senders.end()) {
      if (src != old.src) {
        printf("Previous matching message from %s.\n", old.src.toString().c_str());
      }

      senders.push_back(old.src);
      matches.push_back(old);
      n_found++;
      iter++;
    }
    // If neither of the above, keep looking.
    else {
      iter++;
    }
  }

  computeDesignatedPoint(matches);

  return n_found >= n_autorefs;
}
