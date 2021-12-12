#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void insert_list(struct Node **head, const char* route, const char* file) {
    struct Node *node = (struct Node*) malloc(sizeof(struct Node));
    node->next = NULL;
    node->route = (char *) malloc(strlen(route) + 1);
    node->file = (char *) malloc(strlen(file) + 1);
    if (node == NULL 
        || node->route == NULL 
        || node->file == NULL) 
    { // fail to allocate enough memory
        fprintf(stderr, "Cannot allocate a memory.\n");
        free(node->file);
        free(node->route);
        free(node);
        exit(1);
    }
    strcpy(node->route, route);
    strcpy(node->file, file);
    if (*head == NULL) {
        *head = node;
        return;
    }
    struct Node *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;
}

void clean_list(struct Node **head) {
    struct Node *node = *head;
    while (node) {
        struct Node *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

struct Node* find_route(struct Node* head, const char* route) {
    struct Node *node = head;
    while (node) {
        if (!strcmp(node->route, route)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}
