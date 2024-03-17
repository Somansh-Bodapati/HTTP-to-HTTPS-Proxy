#include "encryption.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "fcntl.h"

#define BUFFER_SIZE 4096

void perform_https_request(const char *method1, const char *uri, const char *httpVersion, const char *hostname, int port, const char *request, int client_sockfd) {
    //printf("Starting HTTPS request to %s:%d\n", hostname, port);

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    const SSL_METHOD *method = SSLv23_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Resolve hostname to IP address
    struct hostent *host = gethostbyname(hostname);
    if (!host) {
        herror("gethostbyname failed");
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Failed to create socket");
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    // Connect to host
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to connect to host");
        send_http_error_response(sockfd, 504, "Gateway Timeout");
        log_request(method1, uri, httpVersion, 504, 0);
        close(sockfd);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    printf("TCP connection established\n");

    // Set up SSL
    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        ERR_print_errors_fp(stderr);
        close(sockfd);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) != 1) {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }
    printf("SSL handshake completed\n");

    // Send request
    if (SSL_write(ssl, request, strlen(request)) <= 0) {
        ERR_print_errors_fp(stderr);
    }
    printf("Request sent, waiting for response...\n");

    // Receive and forward response
    char buffer[4096];
    int bytes;
    int responseCode = 0;
    size_t totalBytes = 0; // Total response size (headers + body)

// Set a timeout for SSL_read
    struct timeval timeout;
    timeout.tv_sec = 5;  // 5 seconds timeout
    timeout.tv_usec = 0;

// Prepare the file descriptor set
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sockfd, &fds);

    int closeConnection = 0;  // Flag to indicate if "Connection: close" header was found

// Read the first chunk to get the response code and headers
    bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0'; // Null-terminate the buffer
        totalBytes += bytes; // Add the bytes count to total response size

        // Extract the response code from the HTTP response line
        sscanf(buffer, "HTTP/%*s %d", &responseCode);

        // Check for "Connection: close" header
        if (strstr(buffer, "Connection: close") != NULL) {
            closeConnection = 1;
        }

        // Forward the initial part of the response to the client
        write(client_sockfd, buffer, bytes);
    }

// Continue reading and forwarding the rest of the response
    while (closeConnection == 0 && select(sockfd + 1, &fds, NULL, NULL, &timeout) > 0) {
        bytes = SSL_read(ssl, buffer, sizeof(buffer));
        if (bytes > 0) {
            write(client_sockfd, buffer, bytes);
            totalBytes += bytes; // Keep adding the bytes count to total response size
        } else {
            break;  // No more data to read or an error occurred
        }
    }

    if (bytes < 0) {
        int err = SSL_get_error(ssl, bytes);
        if (err != SSL_ERROR_ZERO_RETURN && err != SSL_ERROR_WANT_READ) {
            ERR_print_errors_fp(stderr);
        }
    }

    // Log the request and response details
    log_request(method1, uri, httpVersion, responseCode, totalBytes);


    // Clean up
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    printf("HTTPS request processing completed\n");
}
