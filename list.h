#ifndef LIST_H__
#define LIST_H__

struct Node {
    char *path;
    char *file;
    struct Node *next;
};

void Insert(struct Node **head, const char* path, const char* file);

void Clean(struct Node **head);

struct Node* Find(struct Node* head, const char* path);

#endif // LIST_H__
