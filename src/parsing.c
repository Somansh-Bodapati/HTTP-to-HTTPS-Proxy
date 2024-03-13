#include "parsing.h"

#include "parsing.h"
#include <stdlib.h>
#include <string.h>
#include "stdio.h"
#include <unistd.h>
#include <sys/socket.h>

#define BUFFER_SIZE 4096  // Adjust according to your needs

// Initialize HttpRequest struct to zero
void initHttpRequest(HttpRequest *req) {
    memset(req, 0, sizeof(HttpRequest));
    req->port = 443;  // Default to HTTPS port
}

// Helper function to extract host and port from URI
void extract_host_port(const char *uri, HttpRequest *req) {
    // Simple check to see if URI starts with http:// (not handling https here because it's assumed the proxy does the conversion)
    if (strncmp(uri, "http://", 7) == 0) {
        const char *host_start = uri + 7;  // Move past "http://"
        const char *port_or_path = strchr(host_start, ':');  // Check if there is a port specified

        if (port_or_path) {
            // Port is specified
            strncpy(req->host, host_start, port_or_path - host_start);  // Copy host part
            req->port = atoi(port_or_path + 1);  // Convert port number to integer
        } else {
            // No port specified, look for the start of the path
            port_or_path = strchr(host_start, '/');
            if (port_or_path) {
                strncpy(req->host, host_start, port_or_path - host_start);  // Copy host part
            } else {
                strcpy(req->host, host_start);  // Entire remaining part is the host
            }
        }
    }
}


HttpRequest parse_http_request(int sockfd) {
    HttpRequest req;
    initHttpRequest(&req);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_read <= 0) {
        perror("recv failed or connection closed");
        return req;  // Return the empty (zeroed) request struct
    }

    buffer[bytes_read] = '\0';  // Null-terminate the buffer

    // Parse the start line
    sscanf(buffer, "%s %s %s", req.method, req.uri, req.httpVersion);

    // Further parsing can be done here to extract headers

    // Extract host and port from URI
    extract_host_port(req.uri, &req);

    return req;
}

