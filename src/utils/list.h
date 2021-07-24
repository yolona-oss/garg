#ifndef _LIST_H_
#define _LIST_H_

/* structs */
typedef struct Node node_t;
struct Node {
	struct Node *next;
	char *value;
};

/* func */
void list_freeall(node_t *head);

node_t *list_new(char *val);
node_t *list_addhead(node_t **head, node_t *newp);
node_t *list_rmname(node_t *head, char *name);
node_t *list_rmhead(node_t *head);

void list_apply(node_t *head, void (*fn)(node_t *, void *), char *arg);

#endif