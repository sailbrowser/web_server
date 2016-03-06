#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "server.h"

#define LOG_FILENAME  "./server.log"


// static void *threadFunc(void *arg)
// {
//   return NULL;
// }

int main(int argc, char *argv[])
{
  int opt;
  char *ip = (char *)"127.0.0.1";
  unsigned port = 8080;
  char * dir;
  while((opt = getopt(argc, argv, "h:p:d:")) != -1){
      switch (opt){
      case 'h':
        ip = optarg;
        break;
      case 'p':
        port = strtol(optarg, NULL, 10);
        if(port == 0) {
          std::cerr << "Error: incorrect port = " << optarg << std::endl;
          exit(EXIT_FAILURE);
        }
        break;
      case 'd':
        dir = optarg;
        break;
      default:
        std::cerr << "Usage: " << argv[0] << " -h <ip> -p <port> -d <directory>" << std::endl;
        exit(EXIT_FAILURE);
      };
    };
  if(optind < 7) {
    std::cout << "Usage: " << argv[0] << " -h <ip> -p <port> -d <directory>" << std::endl;
    exit(EXIT_FAILURE);
  }

  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) {
    perror("daemon fork");
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
  // int fd_log = open(LOG_FILENAME, O_WRONLY | O_CREAT | O_APPEND, 0666);
  // if(fd_log == -1) {
  //   perror("open log");
  //   exit(EXIT_FAILURE);
  // }
  // if(dup2(fd_log, STDOUT_FILENO) == -1) {
  //   perror("dup2 stdout");
  //   exit(EXIT_FAILURE);
  // };
  // if(dup2(fd_log, STDERR_FILENO) == -1) {
  //   perror("dup2 stderr");
  //   exit(EXIT_FAILURE);
  // }

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
  close(STDOUT_FILENO);
  // close(fd_log);
  // close(STDERR_FILENO);

  /* Daemon-specific initialization goes here */
  // std::cout << "test stdout log" << std::endl ;
  // std::cerr << "test stderr log" << std::endl;
  /* The Big Loop */
  // while (1) {
  //     Do some task here ...
  //    sleep(30); /* wait 30 seconds */
  // }
  int num_cores = get_num_cores();
  pid_t *worker_pid = new pid_t[num_cores];
  int *worker_sv = new int[num_cores];

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
      sleep(1);
      worker(sv[1], dir);
      exit(EXIT_SUCCESS);
    } else { // parent
      close(sv[1]);
      worker_sv[i] = sv[0];
    }
  }
  // std::cout << " --- start: port=" << port << " ip=" << ip
  //   << " dir=" << dir << " cores=" << num_cores << "---" << std::endl;
  server(port, ip, worker_sv, num_cores);

  int status;
  for (int i = 0; i < num_cores; ++i)
  {
    wait(&status); // prevent zombies
    close(worker_sv[i]);
  }
  // // fake thread
  //   pthread_t thread;
  //  int status_addr;
  //  int arg = 42;
  //  int result=pthread_create(&thread, NULL, threadFunc, &arg);
  //  if(result != 0) {
  //    perror("pthread_create");
  //    exit(1);
  //  }
  //  int exit_status = pthread_join(thread, (void**)&status_addr);
  //  if (exit_status != 0) {
  //      perror("pthread_join");
  //      exit(1);
  //  }
  //  // end fake thread
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  delete[] worker_pid;
  delete[] worker_sv;
  return EXIT_SUCCESS;
}
