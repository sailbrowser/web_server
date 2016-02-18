#include <iostream>
#include <unistd.h>
#include <stdlib.h>

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
  return 0;
}
