#ifndef HTTP_TO_HTTPS_PROXY_ENCRYPTION_H
#define HTTP_TO_HTTPS_PROXY_ENCRYPTION_H

#include "parsing.h"
#include "proxy.h"
#include "connection.h"

void perform_https_request(const char *method, const char *uri, const char *httpVersion, const char *hostname, int port, const char *request, int client_sockfd);

#endif //HTTP_TO_HTTPS_PROXY_ENCRYPTION_H
