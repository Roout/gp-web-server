#ifndef LIST_H__
#define LIST_H__

typedef struct {
    char *path;
    char *file;
    Node *next;
} Node;

void Insert(Node **head, const char* path, const char* file);

void Clean(Node **head);

Node* Find(Node* head, const char* path);

#endif // LIST_H__