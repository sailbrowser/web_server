#include <unistd.h>

int get_num_cores() {
// #ifdef _SC_NPROCESSORS_ONLN
  return sysconf(_SC_NPROCESSORS_ONLN);
// #endif
}
