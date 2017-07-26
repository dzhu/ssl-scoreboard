#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

#include "shared/autoref_comparer.h"
#include "shared/rconclient.h"
#include "shared/udp.h"
#include "shared/util.h"

#include "ssl_autoref.pb.h"
#include "ssl_referee.pb.h"

int main(int argc, char **argv)
{
  puts("Autoref comparer running...");

  int n_autorefs = 1;
  uint64_t time_thresh_msec = 1000;

  if (argc > 1) {
    n_autorefs = atoi(argv[1]);
  }
  if (argc > 2) {
    n_autorefs = atol(argv[2]);
  }

  UDP autoref_net;
  if (!autoref_net.open(AutorefGroup, AutorefPort, false)) {
    puts("Autoref port open failed!");
  }

  ssl::SSL_Autoref msg;
  fd_set read_fds;
  int n_fds = 1 + autoref_net.getFd();
  struct timeval timeout;
  Address src;

  AutorefComparer comparer(n_autorefs, time_thresh_msec * 1000);

  bool rcon_opened = false;
  RemoteClient rcon;

  while (true) {
    FD_ZERO(&read_fds);
    FD_SET(autoref_net.getFd(), &read_fds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 50000;
    select(n_fds, &read_fds, nullptr, nullptr, &timeout);

    if (FD_ISSET(autoref_net.getFd(), &read_fds) && autoref_net.recv(msg, src)) {
      if (comparer.proc_msg(msg, src)) {
        if (!rcon_opened) {
          rcon_opened = rcon.open("localhost", 10007);
        }

        // TODO actually decide on the right command
        rcon.sendCommand(SSL_Referee::STOP);
      }
    }
  }
}
