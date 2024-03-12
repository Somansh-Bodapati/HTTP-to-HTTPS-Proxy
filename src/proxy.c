#include "proxy.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Invalid argument. Usage : %s <listen_port> <forbidden_file_path> <access_log_path>\n",
                argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    const char *forbidden_path = argv[2];
    const char *access_path = argv[3];
    fprintf(stdout, "Port: %d\n", port);
    fprintf(stdout, "Forbidden path: %s\n", forbidden_path);
    fprintf(stdout, "Access path: %s\n", access_path);

    //Check if port number and droppc are valid
    if (port < 1 || port > 65535) {
        printf("Invalid Port\n");
    }
    return 0;
}
