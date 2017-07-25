#pragma once

#include "shared/udp.h"
#include "ssl_autoref.pb.h"

const int64_t TIME_THRESH_USEC = 1000 * 1000;

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

  bool autoref_msg_equal(const ssl::SSL_Autoref &p1, const ssl::SSL_Autoref &p2);

public:
  AutorefComparer() : n_autorefs(1)
  {
  }

  AutorefComparer(int n_autorefs) : n_autorefs(n_autorefs)
  {
  }

  void setNumAutorefs(int n)
  {
    n_autorefs = n;
  }

  bool proc_msg(const ssl::SSL_Autoref &msg, const Address &src);
};
