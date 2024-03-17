#ifndef HTTP_TO_HTTPS_PROXY_PARSING_H
#define HTTP_TO_HTTPS_PROXY_PARSING_H


#include "proxy.h"
#include "connection.h"
#include "encryption.h"


// Define a structure to hold the parsed components of the HTTP request
typedef struct {
    char method[8];  // Method should be sufficient to hold "GET" or "HEAD"
    char uri[2048];  // Large enough for most URIs
    char httpVersion[16]; // To hold something like "HTTP/1.1"
    char host[1024];  // To store the host part of the URI
    int port;  // To store the port if specified, otherwise default to 443 for HTTPS
} HttpRequest;

// Function prototype to parse the HTTP request
HttpRequest parse_http_request(char *buffer);

void initHttpRequest(HttpRequest *req);

void extract_host_port(const char *uri, HttpRequest *req);

void send_http_error_response(int sockfd, int status_code, const char* message);

void adjust_request_for_https(const char *http_request, char *https_request, const HttpRequest *req);

#endif //HTTP_TO_HTTPS_PROXY_PARSING_H
