#ifndef HTTP_TO_HTTPS_PROXY_PROXY_H
#define HTTP_TO_HTTPS_PROXY_PROXY_H

#include "connection.h"
#include "parsing.h"
#include "encryption.h"
#include "stdbool.h"

typedef struct ListNode {
    char* ip;
    struct ListNode* next;
} ListNode;

void error(const char *msg);

void log_request(const char* method, const char* uri, const char* httpversion, int statusCode, int responseSize);

int is_ip_forbidden(ListNode* head, const char* ip);

void insert_ip(ListNode** head, const char* ip);

void load_forbidden_sites(ListNode** head, const char* filename);

void free_list(ListNode** head);

void sigint_handler(int sig);

void *handle_client(void *newsockfd);


#endif //HTTP_TO_HTTPS_PROXY_PROXY_H
