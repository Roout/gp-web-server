#ifndef LIST_H__
#define LIST_H__

struct Node {
    char *route;
    char *file;
    struct Node *next;
};

void insert_list(struct Node **head, const char* route, const char* file);

void clean_list(struct Node **head);

struct Node* find_route(struct Node* head, const char* route);

#endif // LIST_H__
