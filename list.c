#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "eprintf.h"

void
list_freeall(node_t *head)
{
	node_t *next;
	for ( ; head != NULL; head = next) {
		next = head->next;
		free(head);
	}
}

node_t *
list_new(char *val)
{
	node_t *newp;
	newp = (node_t *)emalloc(sizeof*newp);

	newp->value = estrdup(val);
	newp->next  = NULL;

	return newp;
}

node_t *
list_addhead(node_t **head, node_t *new)
{
	new->next = *head;
	return new;
}

node_t *
list_rmname(node_t *head, char *name)
{
	node_t *p, *prev;

	prev = NULL;
	for (p = head; p != NULL; p = p->next) {
		if (strcmp(name, p->value) == 0) {
			if (prev == NULL)
				head = p->next;
			else
				prev->next = p->next;
			free(p);
			return head;
		}
		prev = p;
	}

	warn("delitem: %s not in list", name);
	return NULL;
}

node_t *
list_rmhead(node_t *head)
{
	free(head->value);
	node_t *next = head->next;
	head = next;

	return head;
}

void
list_apply(node_t *head, void (*fn)(node_t *, void *), char *arg)
{
	for (; head != NULL; head = head->next) {
		(*fn)(head, arg);
	}
}
