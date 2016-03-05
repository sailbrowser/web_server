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

#define BUFF_SIZE 4096
#define MAX_PATH_LEN 1024

const char* http_get_msg = "GET /";
const char* success_msg = "HTTP/1.0 200 OK\n";
const char* not_found_msg = "HTTP/1.0 404 Not found\n";
const char* content_type_msg = "Content-Type: text/html\n";

struct http_io {
  struct ev_io w_io;
  char *dir;
  // char const *tmp;
};

void read_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  struct stat stat_buf;
  char buffer[BUFF_SIZE];
  char path[MAX_PATH_LEN];

  ssize_t r = recv(watcher->fd, buffer, sizeof(buffer), MSG_NOSIGNAL);
  if (r < 0) {
    perror("read:");
    return;
  }
  // struct http_io *w_read = (struct http_io *)watcher;
  printf("%s\n", buffer);
  if(strncmp(http_get_msg, buffer, strlen(http_get_msg)) == 0) {
    strncpy(path, "index.html", sizeof(path) - 1);
    // strncat(path, "/index.html", sizeof(path) - 1);
    int fd = open(path, O_RDONLY);
    if(fd == -1) {
      send(watcher->fd, not_found_msg, strlen(not_found_msg), MSG_NOSIGNAL);
    } else {
      send(watcher->fd, success_msg, strlen(success_msg), MSG_NOSIGNAL);
      send(watcher->fd, content_type_msg, strlen(content_type_msg), MSG_NOSIGNAL);
      /* get the size of the file to be sent */
      fstat(fd, &stat_buf);
      // int offset = 0;
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
// Read accepted socked from main process
//
void socket_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
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
    ev_run(loop, 0);
    // shutdown(sv, SHUT_RDWR);
    close(sv);
}
