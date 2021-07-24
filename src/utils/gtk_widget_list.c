#include <stdlib.h>
#include <string.h>

#include "gtk_widget_list.h"
#include "eprintf.h"

gw_node_t *
gw_list_init(GtkWidget *w)
{
	gw_node_t *node = (gw_node_t *)emalloc(sizeof*node);
	node->next = NULL;
	node->widget = w;

	return node;
}

gw_node_t *
gw_list_add_item(gw_node_t *node, GtkWidget *w)
{
	gw_node_t *p, *tmp;
	tmp = (gw_node_t *)emalloc(sizeof*tmp);
	p = node->next;
	node->next = tmp;
	tmp->widget = w;
	tmp->next = p;

	return tmp;
}

gw_node_t *
gw_list_rm_item(gw_node_t *wanted, gw_node_t *head)
{
	gw_node_t *tmp;
	tmp = head;
	while (tmp->next != wanted) {
		tmp = tmp->next;
	}
	tmp->next = wanted->next;
	free(tmp);

	return tmp;
}

gw_node_t *
gw_list_rm_head(gw_node_t *head)
{
	gw_node_t *tmp;
	tmp = head->next;
	free(head);
	return tmp;
}

gw_node_t * 
gw_list_swap_items(gw_node_t *n_src, gw_node_t *n_dst, gw_node_t *head)
{
	gw_node_t *prev1, *prev2, *next1, *next2;
	prev1 = head;
	prev2 = head;
	if (prev1 == n_src) {
		prev1 = NULL;
	} else {
		while (prev1->next != n_src) {
			prev1 = prev1->next;
		}
	}

	if (prev2 == n_dst) {
		prev2 = NULL;
	} else {
		while (prev2->next != n_dst) {
			prev2 = prev2->next;
		}
	}

	next1 = n_src->next;
	next2 = n_dst->next;
	if (n_dst == next1) {
		n_dst->next = n_src;
		n_src->next = next2;
		if (n_src != head)
			prev1->next = n_dst;
	} else {
		if (n_src == next2) {
			n_src->next = n_dst;
			n_dst->next = next1;
			if (n_dst != head) {
				prev2->next = n_dst;
			}
		} else {
			if (n_src != head) {
				prev1->next = n_dst;
			}
			n_dst->next = next1;
			if (n_dst != head) {
				prev2->next = n_src;
			}
			n_src->next = next2;
		}
	}
	if (n_src == head)
		return(n_dst);
	if (n_dst == head)
		return(n_src);
	return(head);
}

void
gw_list_freeall(gw_node_t *head)
{
	gw_node_t *next;
	for ( ; head != NULL; head = next) {
		next = head->next;
		free(head);
	}
}

void
gw_list_apply(gw_node_t *head, void (*fn)(gw_node_t *, void *), char *arg)
{
	for (; head != NULL; head = head->next) {
		(*fn)(head, arg);
	}
}
