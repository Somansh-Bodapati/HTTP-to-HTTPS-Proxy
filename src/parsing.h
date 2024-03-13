#ifndef HTTP_TO_HTTPS_PROXY_PARSING_H
#define HTTP_TO_HTTPS_PROXY_PARSING_H


#include "proxy.h"
#include "connection.h"


typedef struct {
    char method[8];
    char uri[2048];
    char httpVersion[16];
    // Add more fields as needed for headers and body
} HttpRequest;

HttpRequest parse_http_request(int sockfd);


#endif //HTTP_TO_HTTPS_PROXY_PARSING_H
