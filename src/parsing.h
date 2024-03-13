#ifndef HTTP_TO_HTTPS_PROXY_PARSING_H
#define HTTP_TO_HTTPS_PROXY_PARSING_H


#include "proxy.h"
#include "connection.h"


// Define a structure to hold the parsed components of the HTTP request
typedef struct {
    char method[8];  // Method should be sufficient to hold "GET" or "HEAD"
    char uri[2048];  // Large enough for most URIs
    char httpVersion[16]; // To hold something like "HTTP/1.1"
    char host[1024];  // To store the host part of the URI
    int port;  // To store the port if specified, otherwise default to 443 for HTTPS
} HttpRequest;

// Function prototype to parse the HTTP request
HttpRequest parse_http_request(int sockfd);



#endif //HTTP_TO_HTTPS_PROXY_PARSING_H
