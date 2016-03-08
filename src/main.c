#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>

#include "server.h"

// #define LOG_FILENAME  "./server.log"

int main(int argc, char *argv[])
{
  int opt;
  char *ip = (char *)"127.0.0.1";
  unsigned port = 8080;
  char * dir = "./";
  while((opt = getopt(argc, argv, "h:p:d:")) != -1){
      switch (opt){
      case 'h':
        ip = optarg;
        break;
      case 'p':
        port = strtol(optarg, NULL, 10);
        if(port == 0) {
          printf("Error: incorrect port = %s\n", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      case 'd':
        dir = optarg;
        break;
      default:
      printf("Usage: %s -h <ip> -p <port> -d <directory>", argv[0]);
        exit(EXIT_FAILURE);
      };
    };
  if(optind < 7) {
    printf("Usage: %s -h <ip> -p <port> -d <directory>", argv[0]);
    exit(EXIT_FAILURE);
  }

  // disable SIGHUP for online test
  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_RESTART;
  sigfillset(&sa.sa_mask);
  sigaction(SIGHUP, &sa, NULL);

  demonize(dir);

  int num_cores = get_num_cores();
  pid_t worker_pid[num_cores];
  int worker_sv[num_cores];

  for (int i = 0; i < num_cores; ++i)
  {
    int sv[2];
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0){
      perror("socketpair");
      exit(1);
    }
    if((worker_pid[i] = fork()) < 0) {
      perror("fork");
      exit(1);
    } else if(worker_pid[i] == 0) { // child
      close(sv[0]);
      // sleep(1);
      worker(sv[1], dir);
      exit(EXIT_SUCCESS);
    } else { // parent
      close(sv[1]);
      worker_sv[i] = sv[0];
    }
  }
  server(port, ip, worker_sv, num_cores);

  int status;
  for (int i = 0; i < num_cores; ++i)
  {
    wait(&status); // prevent zombies
    close(worker_sv[i]);
  }

  return EXIT_SUCCESS;
}
