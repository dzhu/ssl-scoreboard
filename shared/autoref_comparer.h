#pragma once

#include "shared/udp.h"
#include "ssl_autoref.pb.h"

struct received_msg
{
  uint64_t time;
  ssl::SSL_Autoref msg;
  Address src;
};

class AutorefComparer
{
  std::vector<received_msg> history;
  int n_autorefs;
  uint64_t time_thresh_usec;
  bool has_designated_point;
  SSL_Referee::Point designated_point;
  std::vector<ssl::SSL_Autoref> matching_messages;

  bool autoref_msg_equal(const ssl::SSL_Autoref &p1, const ssl::SSL_Autoref &p2);

  void computeDesignatedPoint(std::vector<received_msg> msgs);

public:
  AutorefComparer(int n = 1, uint64_t t = 1000 * 1000) : n_autorefs(n), time_thresh_usec(t), has_designated_point(false)
  {
  }

  void setNumAutorefs(int n)
  {
    n_autorefs = n;
  }

  void setTimeThresh(uint64_t t)
  {
    time_thresh_usec = t;
  }

  bool proc_msg(const ssl::SSL_Autoref &msg, const Address &src);

  bool hasDesignatedPoint() const
  {
    return has_designated_point;
  }

  std::vector<ssl::SSL_Autoref> getMatchingMessages() const
  {
    return matching_messages;
  }

  SSL_Referee::Point getDesignatedPoint() const
  {
    return designated_point;
  }
};
