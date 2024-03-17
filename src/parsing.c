#include "parsing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include "stdbool.h"

#define BUFFER_SIZE 4096

void initHttpRequest(HttpRequest *req) {
    memset(req, 0, sizeof(HttpRequest));
    req->port = 443; // Default HTTPS port
}

void send_http_error_response(int sockfd, int status_code, const char* status_message) {
    char response[1024];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: text/html\r\n"
             "Connection: close\r\n"
             "\r\n"
             "<html><head><title>%d %s</title></head>"
             "<body><h1>%d %s</h1></body></html>\r\n",
             status_code, status_message,
             status_code, status_message,
             status_code, status_message);

    write(sockfd, response, strlen(response));
}

void extract_host_port(const char *uri, HttpRequest *req) {
    if (strncmp(uri, "http://", 7) == 0) {
        const char *host_start = uri + 7; // Skip "http://"
        const char *port_or_path = strchr(host_start, ':');

        if (port_or_path) {
            size_t host_length = port_or_path - host_start;
            memcpy(req->host, host_start, host_length);
            req->host[host_length] = '\0'; // Ensure null-termination
            req->port = atoi(port_or_path + 1);
        } else {
            port_or_path = strchr(host_start, '/');
            size_t host_length = (port_or_path ? port_or_path - host_start : strlen(host_start));
            memcpy(req->host, host_start, host_length);
            req->host[host_length] = '\0'; // Ensure null-termination
        }
    }
}

void adjust_request_for_https(const char *http_request, char *https_request, const HttpRequest *req) {
    const char *path_start = strchr(req->uri + 7, '/');
    if (!path_start) {
        path_start = "/";  // Default to root if no path found
    }

    int written = snprintf(https_request, BUFFER_SIZE, "GET %s HTTP/1.1\r\n", path_start);
    if (written < 0 || written >= BUFFER_SIZE) {
        // Handle error or truncation
    }

    size_t remaining = BUFFER_SIZE - written;
    if (req->port == 443 || req->port == 0) {  // Default HTTPS port or unspecified
        written += snprintf(https_request + written, remaining, "Host: %s\r\n", req->host);
    } else {
        written += snprintf(https_request + written, remaining, "Host: %s:%d\r\n", req->host, req->port);
    }
    if (written < 0 || written >= BUFFER_SIZE) {
        // Handle error or truncation
    }

    remaining = BUFFER_SIZE - written;
    const char *current_pos = strstr(http_request, "\r\n") + 2;  // Start of headers
    while ((current_pos = strstr(current_pos, "\r\n")) != NULL) {
        current_pos += 2;  // Skip "\r\n"

        if (strncasecmp(current_pos, "Host:", 5) == 0 || strncasecmp(current_pos, "Proxy-Connection:", 17) == 0) {
            continue;  // Skip "Host" and "Proxy-Connection" headers
        }

        const char *line_end = strstr(current_pos, "\r\n");
        if (!line_end) {
            break;  // End of headers
        }

        size_t line_length = line_end - current_pos + 2;  // Include "\r\n"
        if (line_length < remaining) {
            strncat(https_request, current_pos, line_length);
            written += line_length;
            remaining -= line_length;
        } else {
            // Handle truncation
            break;
        }
    }

    if (remaining > 2) {
        strcat(https_request, "\r\n");  // Finalize headers
    } else {
        // Handle truncation
    }
}

// In your perform_https_request function, make sure you're properly handling the SSL connection and response forwarding.


HttpRequest parse_http_request(char *buffer) {
    HttpRequest req;
    initHttpRequest(&req);

    printf("Original Request: %s\n", buffer);

    sscanf(buffer, "%7s %2047s %15s", req.method, req.uri, req.httpVersion);

    // If there are errors in the request (you'll need additional checks here)
    // send_http_error_response(sockfd, 400, "Bad Request");

    extract_host_port(req.uri, &req);

    return req;
}
