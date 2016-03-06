#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ev.h>
#include <sys/sendfile.h>

#include "server.h"
#include "socket_fd.h"
#include "request.h"
#include "response.h"

// #define BUFF_SIZE 4096
// #define MAX_PATH_LEN 1024

// const char* http_get_msg = "GET /";
// const char* success_msg = "HTTP/1.0 200 OK\n";
// const char* not_found_msg = "HTTP/1.0 404 Not found\n";
// const char* content_type_msg = "Content-Type: text/html\n";

struct http_io {
  struct ev_io w_io;
  char *dir;
  // char const *tmp;
};

static void workersigterm_cb (struct ev_loop *loop, struct ev_signal *w, int revents)
{
  ev_unloop (loop, EVUNLOOP_ALL);
}

static void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  int fd;
  struct stat stat_buf;
  char buffer[BUFF_SIZE];
  // char path[MAX_PATH_LEN];
  struct http_request req;
  http_request_init(req);
  struct http_response res;
  http_response_init(res);

  ssize_t r = recv(watcher->fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
  if (r < 0) {
    perror("read:");
    return;
  }
  buffer[r] = 0;
  // struct http_io *w_read = (struct http_io *)watcher;
  server_log("%s\n", buffer);
  if(http_request_parse(req, buffer, r) == -1) {
    res.code = _501;
  } else if(req.method != GET) {
    res.code = _501;
  } else {
    server_log("%s\n", req.path);
    // check file exist and have access to read
    if(access(req.path, F_OK ) != 0) {
      res.code = _404;
    } else {
      fd = open(req.path, O_RDONLY);
      if(fd == -1) {
        res.code = _404;
      } else {
        res.code = _200;
        /* get the size of the file to be sent */
        fstat(fd, &stat_buf);
        res.content_length = stat_buf.st_size;
        res.content_type = html;
      }
    }
    render_header(res);
    server_log("---header---\n%s\n------------\n", res.header);
    send(watcher->fd, res.header, strlen(res.header), MSG_NOSIGNAL);
    if(res.code == _200) {
      ssize_t rc = sendfile (watcher->fd, fd, NULL, stat_buf.st_size);
      if (rc == -1) {
        perror("sendfile");
      }
      close(fd);
    }
  }
  shutdown(watcher->fd, SHUT_RDWR);
  close(watcher->fd);
  ev_io_stop(loop, watcher);
  free(watcher);
}
//
// Read accepted socket from main process
//
static void socket_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  int sd;
  char buf[16];
  if(sock_fd_read(watcher->fd, buf, sizeof(buf), &sd) < 0) {
    perror("sock_fd_read");
    return;
  }
  struct http_io *w_client = (struct http_io *) malloc(sizeof(struct http_io));
  w_client->dir = ((struct http_io *)watcher)->dir;
  ev_io_init(&w_client->w_io, read_cb, sd, EV_READ);
  ev_io_start(loop, &w_client->w_io);
}

void worker(int sv, char *dir) {
    struct ev_loop *loop = ev_default_loop(0);
    if(!loop) {
      perror("ev_default_loop");
      exit(1);
    }
    struct http_io w_socket;
    w_socket.dir = dir;
    ev_io_init(&w_socket.w_io, socket_cb, sv, EV_READ);
    ev_io_start(loop, &w_socket.w_io);

    struct ev_signal signal_watcher;
    ev_signal_init(&signal_watcher, workersigterm_cb, SIGTERM);
    ev_signal_start(loop, &signal_watcher);

    ev_run(loop, 0);
    // shutdown(sv, SHUT_RDWR);
    close(sv);
}
