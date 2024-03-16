#include "encryption.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

void perform_https_request(const char *hostname, int port, const char *request, int client_sockfd) {
    printf("Starting HTTPS request to %s:%d\n", hostname, port);

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
    while ((bytes = SSL_read(ssl, buffer, sizeof(buffer))) > 0) {
        write(client_sockfd, buffer, bytes);
    }

    if (bytes < 0) {
        int err = SSL_get_error(ssl, bytes);
        if (err != SSL_ERROR_ZERO_RETURN) {
            ERR_print_errors_fp(stderr);
        }
    }
    printf("Response forwarded to client\n");

    // Clean up
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    ERR_free_strings();
    printf("HTTPS request processing completed\n");
}
