#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "server.h"

#define LOG_FILENAME  "./server.log"

int main(int argc, char *argv[])
{
  int opt;
  char *ip;
  unsigned port;
  char * dir;
  while((opt = getopt(argc, argv, "h:p:d:")) != -1){
      switch (opt){
      case 'h':
        ip = optarg;
        std::cout << "ip = " << ip << std::endl;
        break;
      case 'p':
        port = strtol(optarg, NULL, 10);
        if(port == 0) {
          std::cout << "Error: incorrect port = " << optarg << std::endl;
          exit(EXIT_FAILURE);
        }
        std::cout << "port = " << port << std::endl;
        break;
      case 'd':
        dir = optarg;
        std::cout << "dir = " << dir << std::endl;
        break;
      default:
        std::cout << "Usage: " << argv[0] << " -h <ip> -p <port> -d <directory>" << std::endl;
        exit(EXIT_FAILURE);
      };
    };
  // std::cout << "Optind = " << optind << std::endl;
  if(optind < 7) {
    std::cout << "Usage: " << argv[0] << " -h <ip> -p <port> -d <directory>" << std::endl;
    exit(EXIT_FAILURE);
  }

  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
          exit(EXIT_FAILURE);
  }
  /* If we got a good PID, then
     we can exit the parent process. */
  if (pid > 0) {
          exit(EXIT_SUCCESS);
  }

  /* Change the file mode mask */
  umask(0);

  /* Open any logs here */
  int fd_log = open(LOG_FILENAME, O_RDWR | O_CREAT, 0666);
  if(fd_log == -1) {
    perror("open log");
    exit(EXIT_FAILURE);
  }
  if(dup2(fd_log, STDOUT_FILENO) == -1) {
    perror("dup2 stdout");
    exit(EXIT_FAILURE);
  };
  if(dup2(fd_log, STDERR_FILENO) == -1) {
    perror("dup2 stderr");
    exit(EXIT_FAILURE);
  }

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    /* Log any failures here */
    exit(EXIT_FAILURE);
  }


  /* Change the current working directory */
  if ((chdir(dir)) < 0) {
    /* Log any failures here */
    exit(EXIT_FAILURE);
  }

  /* Close out the standard file descriptors */
  close(STDIN_FILENO);
  // close(STDOUT_FILENO);
  close(fd_log);
  // close(STDERR_FILENO);

  /* Daemon-specific initialization goes here */
  // std::cout << "test stdout log" << std::endl ;
  // std::cerr << "test stderr log" << std::endl;
  /* The Big Loop */
  // while (1) {
  //     Do some task here ...
  //    sleep(30); /* wait 30 seconds */
  // }
  server(port, ip, dir);
  return 0;
}
