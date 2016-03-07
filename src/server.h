void server(const int port, char const *ip_address, const int *sv, const int cores);
void worker(int sv, char *dir);
int get_num_cores();
void server_log(const char* format, ...);
void demonize(char *dir);
