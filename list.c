#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Insert(struct Node **head, const char* path, const char* file) {
    struct Node *node = (struct Node*) malloc(sizeof(struct Node));
    node->next = NULL;
    node->path = (char *) malloc(strlen(path) + 1);
    node->file = (char *) malloc(strlen(file) + 1);
    if (node == NULL 
        || node->path == NULL 
        || node->file == NULL) 
    { // fail to allocate enough memory
        fprintf(stderr, "Cannot allocate a memory.\n");
        free(node->file);
        free(node->path);
        free(node);
        exit(1);
    }
    strcpy(node->path, path);
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

void Clean(struct Node **head) {
    struct Node *node = *head;
    while (node) {
        struct Node *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

struct Node* Find(struct Node* head, const char* path) {
    struct Node *node = head;
    while (node) {
        if (!strcmp(node->path, path)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}
