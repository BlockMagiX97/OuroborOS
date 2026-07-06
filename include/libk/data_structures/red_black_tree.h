#ifndef OUROBOROS_LIBK_DATA_STRUCTURES_RED_BLACK_TREE_H
#define OUROBOROS_LIBK_DATA_STRUCTURES_RED_BLACK_TREE_H
#include <libk/typedef.h>

enum rb_color { RB_RED, RB_BLACK };
struct rb_node {
	struct rb_node *right;
	struct rb_node *left;
	struct rb_node *parent;
	enum rb_color color;
};
struct rb_tree {
	struct rb_node *root;
};
#define RB_TREE_INIT {.root = NULL}

// if a = b then comp(a,b) = 0, if a > b than comp > 0, if a < b than comp < 0
err_t rb_insert(struct rb_node *node, struct rb_tree *tree,
		int (*comp)(struct rb_node *node_a, struct rb_node *node_b));

// returns null if invalid
// if a = b then comp(a,b) = 0, if a > b than comp > 0, if a < b than comp < 0
// key is used for genericness (doesnt need to be valid, it is only used to pass searched for ptr to comp function)
struct rb_node *rb_search(struct rb_node *key, struct rb_tree *tree,
			  int (*comp)(struct rb_node *node_a, struct rb_node *node_b));

// returns NULL if node is null, else returns minumum in a subtree
struct rb_node *rb_get_min_subtree(struct rb_node *node);

err_t rb_delete(struct rb_node *node, struct rb_tree *tree);
#endif
