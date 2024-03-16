#ifndef HTTP_TO_HTTPS_PROXY_CONNECTION_H
#define HTTP_TO_HTTPS_PROXY_CONNECTION_H

#include "parsing.h"
#include "proxy.h"
#include "encryption.h"

int create_listening_socket(int port);

void accept_connections(int listen_sockfd);

void *handle_client(void *newsockfd);


#endif //HTTP_TO_HTTPS_PROXY_CONNECTION_H
