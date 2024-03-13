#include "connection.h"
#include "sys/socket.h"
#include "arpa/inet.h"
#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "err.h"
#include "unistd.h"

int create_listening_socket(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    if (listen(sockfd, 50) < 0) {  // Listen on the socket, allow 5 pending connections
        error("ERROR on listen");
    }
    return sockfd;
}


void accept_connections(int listen_sockfd) {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    while (1) {
        int *newsockfd = malloc(sizeof(int));
        *newsockfd = accept(listen_sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (*newsockfd < 0) {
            perror("ERROR on accept");
            free(newsockfd);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)newsockfd) < 0) {
            perror("could not create thread");
            free(newsockfd);
        }
        pthread_detach(thread_id);  // The thread resources are automatically released on termination
    }
}

void *handle_client(void *newsockfd) {
    int sockfd = *(int*)newsockfd;
    free(newsockfd);  // Don't forget to free the dynamically allocated memory

    // Handle the client request here
    printf("Handling request\n");
    HttpRequest req = parse_http_request(sockfd);
    printf("Received Request: %s %s %s %s %d\n", req.method, req.uri, req.httpVersion, req.host, req.port);

    close(sockfd);  // Close the client socket
    return NULL;    // Return from the thread
}

