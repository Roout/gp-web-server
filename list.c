#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Insert(Node **head, const char* path, const char* file) {
    Node *node = (Node*) malloc(sizeof(Node));
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
    Node *cur = *head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = node;
}

void Clean(Node **head) {
    Node *node = *head;
    while (node) {
        Node *next = node->next;
        free(node);
        node = next;
    }
    *head = NULL;
}

Node* Find(Node* head, const char* path) {
    Node *node = head;
    while (node) {
        if (!strcmp(node->path, path)) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}