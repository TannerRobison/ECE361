#include <stdint.h>
#include <stdlib.h>

#include "store.h"

struct node {
	record_t record;
	struct node *left;
	struct node *right;
};
struct store {
	struct node *root;
	size_t count;
};

store_t *store_create(void)
{
	store_t *store = malloc(sizeof(store_t));
	if (store == NULL) {
		return NULL;
	}
	store->root = NULL;
	store->count = 0;
	return store;
}

static int rh_key(float rh)
{
	return (int)rh;
}

int record_compare(const record_t *a, const record_t *b)
{
	if (rh_key(a->reading.humidity) == rh_key(b->reading.humidity)) {
		return 0;
	} else if (rh_key(a->reading.humidity) < rh_key(b->reading.humidity)) {
		return -1;
	} else {
		return 1;
	}
}

static struct node *create_node(const record_t *record)
{
	struct node *n = malloc(sizeof(struct node));
	if (n == NULL) {
		return NULL;
	}
	n->record = *record;
	n->left = NULL;
	n->right = NULL;
	return n;
}

int store_insert(store_t *store, const record_t *new_record)
{
	if (store == NULL || new_record == NULL) {
		return -1;
	}

	// create the node
	struct node *n = create_node(new_record);
	if (n == NULL) {
		return -1;
	}

	// if the tree is empty, node is new root
	if (store->root == NULL) {
		store->root = n;
	}
	else {
		// loop through the tree until find NULL node pointer
		struct node *cur = store->root;
		while (1) {
			// go left
			if (record_compare(new_record, &cur->record) < 0) {
				if (cur->left == NULL) {
					cur->left = n;
					break;
				}
				// go down a level (left)
				else {
					cur = cur->left;
				}
			}

			// go right
			else {
				if (cur->right == NULL) {
					cur->right = n;
					break;
				}
				// go down a level (right)
				else {
					cur = cur->right;
				}
			}
		}
	}

	// increment the node counter
	store->count++;
	return 0;
}

size_t store_count(const store_t *store) {
	if (store == NULL) {
		return 0;
	}
	return store->count;
}

static void walk_node(const struct node *node, store_visit_fn callback, void *context) {
	if (node == NULL) {
		return;
	}
	
	//recursively calls itself until reaches null pointer
	walk_node(node->left, callback, context);
	callback(&node->record, context);
	walk_node(node->right, callback, context);
}

void store_walk(const store_t *store, store_visit_fn callback, void *context)
{
	if (store == NULL || callback == NULL) {
		return;
	} 
	walk_node(store->root, callback, context);
}

size_t store_find_by_key(const store_t *store, store_visit_fn callback,
			 void *context, int key)
{
	if (store == NULL) {
		return 0;
	}

	// search down one path, like insert. equal keys are inserted to the
	//  right, so every duplicate of key is below the first match, on its right
	size_t found = 0;
	const struct node *cur = store->root;
	while (cur != NULL) {
		int cur_key = rh_key(cur->record.reading.humidity);
		if (key < cur_key) {
			cur = cur->left;
		} else if (key > cur_key) {
			cur = cur->right;
		} else {
			// match: report it, then keep looking right for duplicates
			if (callback != NULL) {
				callback(&cur->record, context);
			}
			found++;
			cur = cur->right;
		}
	}
	return found;
}

// frees every node below and including node. post-order: both children are
//  freed before the node itself, because once node is freed its left and
//  right pointers can no longer be read
static void destroy_node(struct node *node)
{
	if (node == NULL) {
		return;
	}
	destroy_node(node->left);
	destroy_node(node->right);
	free(node);
}

void store_destroy(store_t *store)
{
	if (store == NULL) {
		return;
	}
	destroy_node(store->root);
	free(store);
}

// number of nodes on the longest path from node down to a leaf; 0 for NULL
static int height_node(const struct node *node)
{
	if (node == NULL) {
		return 0;
	}
	int left = height_node(node->left);
	int right = height_node(node->right);

	// this node counts as 1, plus the taller of its two sides
	int taller;
	if (left > right) {
		taller = left;
	} else {
		taller = right;
	}
	return 1 + taller;
}

int store_height(const store_t *store)
{
	if (store == NULL) {
		return 0;
	}
	return height_node(store->root);
}
