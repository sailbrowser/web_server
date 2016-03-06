#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ev.h>
#include <netdb.h>

#include "server.h"
#include "socket_fd.h"


struct socket_io {
  struct ev_io w_io;
  int cores;
  const int *sv;
};

static void sigterm_cb (struct ev_loop *loop, struct ev_signal *w, int revents)
{
  ev_unloop (loop, EVUNLOOP_ALL);
}

static void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  int client_sd = accept(watcher->fd, 0, 0);
  if(client_sd == -1) {
    perror("accept");
    return;
  }
  struct socket_io *w_accept = (struct socket_io *)watcher;
  static int index = 0;
  int sv = w_accept->sv[index++];
  index %= w_accept->cores;
  // send socket to next worker
  sock_fd_write(sv, (void *)"1", 1, client_sd);
  close(client_sd);
}

void server(const int port, char const *ip_address, const int *sv, const int cores)
{
  struct ev_loop *loop = ev_default_loop(0);
  if(!loop) {
    perror("ev_default_loop");
    exit(1);
  }
  int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sd == -1) {
    perror("socket");
    exit(1);
  }

  struct hostent *he;
  if((he = gethostbyname(ip_address)) == NULL) {
    perror("gethostbyname:");
    exit(1);
  }

  struct sockaddr_in  addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
  // addr.sin_addr.s_addr = htonl(ip_address);
  int res_bind = bind(sd, (struct sockaddr *) &addr, sizeof(addr));
  if(res_bind == -1) {
    perror("bind");
    exit(1);
  }

  int res_listen = listen(sd, SOMAXCONN);
  if(res_listen == -1) {
    perror("listening");
    exit(1);
  }

  struct socket_io w_accept;
  w_accept.cores = cores;
  w_accept.sv = sv;

  ev_io_init(&w_accept.w_io, accept_cb, sd, EV_READ);
  ev_io_start(loop, &w_accept.w_io);

  struct ev_signal signal_watcher;
  ev_signal_init(&signal_watcher, sigterm_cb, SIGTERM);
  ev_signal_start(loop, &signal_watcher);

  ev_run(loop, 0);
  close(sd);
}
