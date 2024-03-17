#include "proxy.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include <netdb.h>
#include <netinet/in.h>
#include "arpa/inet.h"
#include "parsing.h"
#include "time.h"

#define BUFFER_SIZE 4096

// Global variable to store sites
ListNode* forbiddenSites = NULL;
const char* forbidden_path = NULL;
FILE* logFile = NULL;
const char *access_log = NULL;

// Error message and exit
void error(const char *msg) {
    perror(msg);
    exit(1);
}


void log_request(const char* method, const char* uri, const char* httpversion, int statusCode, int responseSize) {
    time_t now = time(NULL);
    char timeStr[32];
    strftime(timeStr, sizeof(timeStr), "%FT%TZ", gmtime(&now));

    fprintf(logFile, "%s %s %s %s %d %d\n", timeStr, method, uri, httpversion, statusCode, responseSize);
    fflush(logFile); // Ensure the log is written immediately
}


void insert_ip(ListNode** head, const char* ip) {
    ListNode* newNode = malloc(sizeof(ListNode));
    newNode->ip = strdup(ip);
    newNode->next = *head;
    *head = newNode;
}



int is_ip_forbidden(ListNode* head, const char* ip) {
    ListNode* current = head;
    while (current) {
        if (strcmp(current->ip, ip) == 0) {
            return 1; // IP address is forbidden
        }
        current = current->next;
    }
    return 0; // IP address is allowed
}



void load_forbidden_sites(ListNode** head, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open forbidden sites file");
        exit(EXIT_FAILURE);
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline character
        struct hostent* he = gethostbyname(line);
        if (he) {
            struct in_addr** addr_list = (struct in_addr**)he->h_addr_list;
            for (int i = 0; addr_list[i] != NULL; i++) {
                insert_ip(head, inet_ntoa(*addr_list[i]));
            }
        } else {
            insert_ip(head, line); // Assume it's already an IP address
        }
    }

    fclose(file);
}

void free_list(ListNode** head) {
    ListNode* current = *head;
    while (current) {
        ListNode* next = current->next;
        free(current->ip);
        free(current);
        current = next;
    }
    *head = NULL;
}



void sigint_handler(int sig) {
    printf("\nCaught signal %d, reloading forbidden sites...\n", sig);
    free_list(&forbiddenSites); // Free the existing list
    load_forbidden_sites(&forbiddenSites, forbidden_path); // Reload
}


void *handle_client(void *newsockfd) {
    int sockfd = *(int*)newsockfd;
    free(newsockfd);  // Don't forget to free the dynamically allocated memory
    // Handle the client request here
    printf("Handling request\n");
    // Load the initial forbidden sites
    load_forbidden_sites(&forbiddenSites, forbidden_path);

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        perror("recv failed or connection closed");
        return NULL;
    }
    HttpRequest req = parse_http_request( buffer);
    printf("Received Request: %s %s %s %s %d\n", req.method, req.uri, req.httpVersion, req.host, req.port);
    // Check for unsupported HTTP methods
    if (strcmp(req.method, "GET") != 0 && strcmp(req.method, "HEAD") != 0) {
        send_http_error_response(sockfd, 501, "Not Implemented");
        log_request(req.method, req.uri, req.httpVersion, 501, 0);
        close(sockfd);
        return NULL;
    }
    if (strlen(req.host) == 0) {
        // The request is invalid; respond with an error
        send_http_error_response(sockfd, 400, "Bad Request");
        log_request(req.method, req.uri, req.httpVersion, 400, 0);
        close(sockfd);
        return NULL;
    }

    // Convert the hostname to an IP address
    struct hostent *he;
    struct in_addr **addr_list;
    char *ip;

    if ((he = gethostbyname(req.host)) == NULL) {
        // Failed to resolve hostname
        herror("gethostbyname");
        send_http_error_response(sockfd, 502, "Bad Gateway");
        log_request(req.method, req.uri, req.httpVersion, 502, 0);
        close(sockfd);
        return NULL;
    }

    // Take the first IP address from the list
    addr_list = (struct in_addr **)he->h_addr_list;
    ip = inet_ntoa(*addr_list[0]);

    // Check if the IP address is forbidden
    if (is_ip_forbidden(forbiddenSites, ip)) {  // Make sure is_ip_forbidden checks against IP addresses
        send_http_error_response(sockfd, 403, "Forbidden");
        log_request(req.method, req.uri, req.httpVersion, 403, 0);
        close(sockfd);
        return NULL;
    }

    char https_request[BUFFER_SIZE] = {0};
    adjust_request_for_https(buffer, https_request, &req);
    printf("Adjusted HTTPS Request: %s\n", https_request);
    perform_https_request(req.method, req.uri, req.httpVersion, req.host, req.port, https_request, sockfd);

    free_list(&forbiddenSites);
    close(sockfd);  // Close the client socket
    return NULL;    // Return from the thread
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Invalid argument. Usage : %s <listen_port> <forbidden_file_path> <access_log_path>\n",
                argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    forbidden_path = argv[2];
    access_log = argv[3];
    logFile = fopen(access_log, "a");


    // Set up the SIGINT handler
    signal(SIGINT, sigint_handler);

//    fprintf(stdout, "Port: %d\n", port);
//    fprintf(stdout, "Forbidden path: %s\n", forbidden_path);
    fprintf(stdout, "Access path: %s\n", access_log);

    //Check if port number and droppc are valid
    if (port < 1 || port > 65535) {
        printf("Invalid Port\n");
    }

    int sockfd = create_listening_socket(port);
    accept_connections(sockfd);

    fclose(logFile);
    return 0;
}

