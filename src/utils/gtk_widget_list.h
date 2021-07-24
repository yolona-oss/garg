#ifndef _GW_LIST_H_
#define _GW_LIST_H_

#include <gtk/gtk.h>
/* structs */
typedef struct gw_node gw_node_t;
struct gw_node {
	GtkWidget *widget;
	struct gw_node *next;
};

/* func */
gw_node_t *gw_list_init(GtkWidget *w);
gw_node_t *gw_list_add_item(gw_node_t *node, GtkWidget *w);
gw_node_t *gw_list_rm_item(gw_node_t *wanted, gw_node_t *head);
gw_node_t *gw_list_rm_head(gw_node_t *head);
gw_node_t *gw_list_swap_items(gw_node_t *n_src, gw_node_t *n_dst, gw_node_t *head);

void gw_list_apply(gw_node_t *head, void (*fn)(gw_node_t *, void *), char *arg);
void gw_list_free_all(gw_node_t *head);

#endif
