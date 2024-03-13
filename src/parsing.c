#include "parsing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 4096

HttpRequest parse_http_request(int sockfd) {
    HttpRequest req;
    char buffer[BUFFER_SIZE];
    char *line;

    // Initialize request structure
    memset(&req, 0, sizeof(HttpRequest));

    // Read the start line
    if (fread(sockfd, buffer, BUFFER_SIZE - 1) <= 0) {
        perror("Failed to read request");
        return req;
    }

    // Parse the start line
    line = strtok(buffer, "\r\n");  // Assuming \r\n as line delimiter
    if (line) {
        sscanf(line, "%s %s %s", req.method, req.uri, req.httpVersion);
    }

    // Parse headers (simplified version)
    while ((line = strtok(NULL, "\r\n")) && strlen(line) > 0) {
        // Here, you would parse and store each header
        printf("Header: %s\n", line);
    }

    // Body handling code should go here if needed

    return req;
}
