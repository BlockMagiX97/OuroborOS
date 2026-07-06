#include <libk/assert.h>
#include <libk/wrap_builtin.h>
#include <libk/typedef.h>
#include <stdbool.h>
#include <stddef.h>

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

void rb_right_rotate(struct rb_node *node, struct rb_tree *tree) {
	struct rb_node *parent = node->parent;
	struct rb_node *left_child = node->left;

	assert(left_child != NULL && "left child is NULL (operation doesnt make sense)");

	node->left = left_child->right;
	if (node->left != NULL) {
		node->left->parent = node;
	}
	left_child->parent = parent;
	if (parent != NULL) {
		if (parent->left == node) {
			parent->left = left_child;
		} else if (parent->right == node) {
			parent->right = left_child;
		} else {
			UNREACHABLE;
		}
	} else {
		tree->root = left_child;
	}
	left_child->right = node;
	node->parent = left_child;
}
void rb_left_rotate(struct rb_node *node, struct rb_tree *tree) {
	struct rb_node *parent = node->parent;
	struct rb_node *right_child = node->right;

	assert(right_child != NULL && "right child is NULL (operation doesnt make sense)");

	node->right = right_child->left;
	if (node->right != NULL) {
		node->right->parent = node;
	}
	right_child->parent = parent;
	if (parent != NULL) {
		if (parent->left == node) {
			parent->left = right_child;
		} else if (parent->right == node) {
			parent->right = right_child;
		} else {
			UNREACHABLE;
		}
	} else {
		tree->root = right_child;
	}
	right_child->left = node;
	node->parent = right_child;
}
// returns null if root is parent, returns node if invalid
// if a = b then comp(a,b) = 0, if a > b than comp > 0, if a < b than comp < 0
static struct rb_node *rb_find_parent(struct rb_node *node, struct rb_tree *tree,
			       int (*comp)(struct rb_node *node_a, struct rb_node *node_b)) {
	struct rb_node *parent = NULL;
	struct rb_node *tmpn = tree->root;
	while (tmpn != NULL) {
		parent = tmpn;
		int res = comp(node, tmpn);
		if (res < 0) {
			tmpn = tmpn->left;
		} else if (res > 0) {
			tmpn = tmpn->right;
		} else {
			// error key already exists
			return node;
		}
	}
	return parent;
}
static enum rb_color get_color(struct rb_node *node) {
	if (node == NULL) {
		return RB_BLACK;
	}
	return node->color;
}
static void rb_fix_insert(struct rb_node *node, struct rb_tree *tree) {
	// case 1 - node is root (theoreticaly when inserting we know this thus this is redundant
	struct rb_node *root = tree->root;
	struct rb_node *parent = node->parent;
	struct rb_node *grand_parent = parent == NULL ? NULL : parent->parent;
	struct rb_node *uncle = grand_parent == NULL ? NULL : (grand_parent->left == parent ? grand_parent->right : grand_parent->left);
	// case 1 - root must be black
	if (node == root) {
		node->color = RB_BLACK;
	} 
	// parent is not null in any other cases (that is possible only on root)
	// case 2 - parent is red and root (technicaly colliding but leave here in case some incorrect rotations mess this up)
	else if (get_color(parent) == RB_RED && parent == root) {
		parent->color = RB_BLACK;
	// case 3 - parent and uncle are red - recolor
	} else if (get_color(parent) == RB_RED && get_color(uncle) == RB_RED) {
		// uncle is not NULL since it is red
		// grandparent is not NULL since uncle is not null
		uncle->color = RB_BLACK;
		parent->color = RB_BLACK;
		grand_parent->color = RB_RED;
		// continue fixing
		rb_fix_insert(grand_parent, tree);
	} else if (get_color(parent) == RB_RED && get_color(uncle) == RB_BLACK) {
		safety_assert(grand_parent != NULL && "dont know if it is possible, so error if it heapens so i can update the code");
		bool is_parent_left_child = grand_parent->left == parent;
		bool is_child_left_child = parent->left == node;
		if  (is_parent_left_child != is_child_left_child) {
			// case 4 - parent is red, uncle black and node is inner grand child
			if (is_child_left_child) {
				rb_right_rotate(parent, tree);
				rb_left_rotate(grand_parent, tree);
			}
			else {
				rb_left_rotate(parent, tree);
				rb_right_rotate(grand_parent, tree);
			}
			node->color = RB_BLACK;
			grand_parent->color = RB_RED;

		} else {
			// case 5 - parent is red, uncle black and node is outer grand child
			if (is_child_left_child) {
				rb_right_rotate(grand_parent, tree);
			} else {
				rb_left_rotate(grand_parent, tree);
			}
			parent->color = RB_BLACK;
			grand_parent->color = RB_RED;
		}
	}
}
// if a = b then comp(a,b) = 0, if a > b than comp > 0, if a < b than comp < 0
err_t rb_insert(struct rb_node *node, struct rb_tree *tree,
	       int (*comp)(struct rb_node *node_a, struct rb_node *node_b)) {
	struct rb_node *parent = rb_find_parent(node, tree, comp);
	if (parent == node) {
		return GENERIC_FAIL;
	} else if (parent == NULL) {
		tree->root = node;
		node->left = NULL;
		node->right = NULL;
		node->parent = NULL;
		node->color = RB_BLACK;
	} else if (comp(node, parent) < 0) {
		parent->left = node;
		node->parent = parent;
		node->left = NULL;
		node->right = NULL;
		node->color = RB_RED;
	} else {
		parent->right = node;
		node->parent = parent;
		node->left = NULL;
		node->right = NULL;
		node->color = RB_RED;
	}
	rb_fix_insert(node, tree);
	return SUCCESS;
}
// returns null if invalid
// if a = b then comp(a,b) = 0, if a > b than comp > 0, if a < b than comp < 0
// key is used for genericness (doesnt need to be valid, it is only used to pass searched for ptr to comp function)
struct rb_node *rb_search(struct rb_node *key, struct rb_tree *tree,
			       int (*comp)(struct rb_node *node_a, struct rb_node *node_b)) {
	struct rb_node *tmpn = tree->root;
	while (tmpn != NULL) {
		int res = comp(key, tmpn);
		if (res < 0) {
			tmpn = tmpn->left;
		} else if (res > 0) {
			tmpn = tmpn->right;
		} else {
			// found the node with correct key
			return tmpn;
		}
	}
	return NULL;
}

// if node is root with no children returns NULL
//  if node had no child:
//  	returns linked in null_node
//  returns moved up node if it has
static struct rb_node *rb_delete_one_or_none_child(struct rb_node *node, struct rb_tree *tree, struct rb_node *null_node) {
	bool has_left_child = node->left != NULL;
	bool has_right_child = node->right != NULL;
	assert(!(has_left_child && has_right_child) && "rb_delete_one_or_none_child called with 2 children");

	struct rb_node *parent = node->parent;

	struct rb_node *child = node->right;
	if (has_left_child)
		child = node->left;

	if (parent == NULL) {
		assert(node == tree->root);
		// we are root
		
		tree->root = child;
		if (child != NULL) {
			child->parent = NULL;
		}
		goto unlink_and_ret;
	}
	bool is_node_left_child = parent->left == node;
	if (child == NULL) {
		// place null_node inside tree to not loose possition link in temporary null_node
		child = null_node;
		if (is_node_left_child)
			parent->left = child;
		else
			parent->right = child;
		null_node->color = RB_BLACK;
		null_node->left = NULL;
		null_node->right = NULL;
		null_node->parent = parent;
	}
	if (parent != NULL) {
		if (is_node_left_child)
			parent->left = child;
		else
			parent->right = child;
		child->parent = parent;
	}
	
unlink_and_ret:
	// unlink the removed node to prevent accidents
	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;

	return child;
}
// returns NULL if node is null, else returns minumum in a subtree
struct rb_node *rb_get_min_subtree(struct rb_node *node) {
	while (node != NULL) {
		if (node->left == NULL) {
			return node;
		}
		node = node->left;
	}
	return NULL;
}
// node must be unlinked from the tree
// node->color doesnt change
static void rb_replace_node_all(struct rb_node *to_replace_node, struct rb_node *node, struct rb_tree *tree) {
	struct rb_node *parent = to_replace_node->parent;
	if (parent == NULL) {
		// to_replace_node is root
		tree->root = node;
	} else {
		bool is_left_child = parent->left == to_replace_node;
		if (is_left_child) {
			parent->left = node;
		} else {
			parent->right = node;
		}
	}
	
	node->parent = parent;
	node->left = to_replace_node->left;
	if (node->left != NULL) {
		node->left->parent = node;
	}
	node->right = to_replace_node->right;
	if (node->right != NULL) {
		node->right->parent = node;
	}

	// unlink it completely to avoid wierd things
	to_replace_node->parent = NULL;
	to_replace_node->left = NULL;
	to_replace_node->right = NULL;
}
static void rb_fix_delete(struct rb_node *node, struct rb_tree *tree) {
	if (node->parent == NULL) {
		// case 1: node is root
		node->color = RB_BLACK;
		return;
	}
	bool is_node_left_child = node->parent->left == node;
	struct rb_node *sibling =  is_node_left_child ? node->parent->right : node->parent->left;
	if (get_color(sibling) == RB_RED) {
		// case 2: sibling is red
		node->parent->color = RB_RED;
		sibling->color = RB_BLACK;
		if (is_node_left_child)
			rb_left_rotate(node->parent, tree);
		else 
			rb_right_rotate(node->parent, tree);
		is_node_left_child = node->parent->left == node;
		sibling =  is_node_left_child ? node->parent->right : node->parent->left;
		// fallthrough
	}
	if (get_color(sibling->left) == RB_BLACK && get_color(sibling->right) == RB_BLACK) {
		// case 3+4: sibling with 2 black children
		sibling->color = RB_RED;
		if (get_color(node->parent) == RB_RED) {
			// case 3: red parent + sibling with 2 black children
			node->parent->color = RB_BLACK;
		} else {
			// case 4: black parent + sibling with 2 black children
			rb_fix_delete(node->parent, tree);
		}
	} else  {
		// at least one of nephews is RED
		bool is_sibling_left_child = !is_node_left_child;
		struct rb_node *outer_nephew = is_sibling_left_child ? sibling->left : sibling->right;
		struct rb_node *inner_nephew = is_sibling_left_child ? sibling->right : sibling->left;
		if (get_color(outer_nephew) == RB_BLACK) {
			// case 5: sibling with at least one red child and outer nephew is black
			inner_nephew->color = RB_BLACK;
			sibling->color = RB_RED;
			if (is_node_left_child)
				rb_right_rotate(sibling, tree);
			else 
				rb_left_rotate(sibling, tree);

			// i hate this
			sibling =  is_node_left_child ? node->parent->right : node->parent->left;
			outer_nephew = is_sibling_left_child ? sibling->left : sibling->right;
			inner_nephew = is_sibling_left_child ? sibling->right : sibling->left;
			// fallthrough to case 6
		}
		// case 6: sibling has at least one red child and outer nephew is red
		sibling->color = node->parent->color;
		node->parent->color = RB_BLACK;
		outer_nephew->color = RB_BLACK;
		if (is_node_left_child)
			rb_left_rotate(node->parent, tree);
		else 
			rb_right_rotate(node->parent, tree);
	}
}
err_t rb_delete(struct rb_node *node, struct rb_tree *tree) {
	struct rb_node *moved_up_node = NULL;
	enum rb_color deleted_color;
	// is needed for positioning
	struct rb_node null_node = {.color = RB_BLACK, .left=NULL, .right=NULL, .parent=NULL};
	if (node->left == NULL || node->right == NULL) {
		moved_up_node = rb_delete_one_or_none_child(node, tree, &null_node);
		deleted_color = node->color;
	} else {
		// 2 children
		struct rb_node *inorder_succesor = rb_get_min_subtree(node->right);
		moved_up_node = rb_delete_one_or_none_child(inorder_succesor, tree, &null_node);

		rb_replace_node_all(node, inorder_succesor, tree);
		deleted_color = inorder_succesor->color;

		inorder_succesor->color = node->color;
	}
	if (tree->root == NULL) {
		return SUCCESS;
	}
	if (deleted_color == RB_BLACK) {
		rb_fix_delete(moved_up_node, tree);
	}
	if (moved_up_node == &null_node) {
		assert(null_node.left == NULL && null_node.right == NULL && "null node shouldnt have children i think, if this triggers, you must update remove node logic");
		// remove null node 
		bool is_left_child = null_node.parent->left == moved_up_node;
		if (is_left_child)
			null_node.parent->left = NULL;
		else
			null_node.parent->right = NULL;
	}
	return SUCCESS;
}

