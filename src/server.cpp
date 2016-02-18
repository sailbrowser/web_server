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
#include <netdb.h>

#define BUFF_SIZE 4096
#define MAX_PATH_LEN 1024

const char* http_get_msg = "GET /";
const char* success_msg = "HTTP/1.0 200 OK\n";
const char* not_found_msg = "HTTP/1.0 404 Not found\n";
const char* content_type_msg = "Content-Type: text/html\n";

struct http_io {
  struct ev_io w_io;
  char const *tmp;
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

void accept_cb(struct ev_loop *loop, struct ev_io *watcher, int revents) {
  int client_sd = accept(watcher->fd, 0, 0);
  if(client_sd == -1) {
    perror("accept");
    return;
  }
  struct http_io *w_client = (struct http_io *) malloc(sizeof(struct http_io));
  // struct http_io *w_accept = (struct http_io *)watcher;
  // w_client->dir = w_accept->dir;
  ev_io_init(&w_client->w_io, read_cb, client_sd, EV_READ);
  ev_io_start(loop, &w_client->w_io);
}

int server(const int port, char const *ip_address, char const *dir)
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

  struct http_io w_accept;
  // w_accept.dir = dir;
  // strncpy(w_accept.dir, dir, sizeof(w_accept.dir));

  ev_io_init(&w_accept.w_io, accept_cb, sd, EV_READ);
  ev_io_start(loop, &w_accept.w_io);
  ev_run(loop, 0);
  return 0;
}
