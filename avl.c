#ifndef avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60
#define avl_guard_3666e1d4b12de3894af37e95950d35fa533fadb5ec91810da57de2f1b22a1f60

#include <stdint.h>
#include <stddef.h>

/* --- TYPES -------------------------------------------------- */

typedef uint32_t avl_key_t;

typedef struct avl_node {
	avl_key_t key;
	struct avl_node *left_son, *right_son, *father;
} avl_node_t;

typedef struct {
	avl_node_t *root_node;
} avl_root_t;

/* --- INTERNAL FUNCTIONS ------------------------------------- */

/* choose next node on the path to the node with given key according to the BST invariant */
static avl_node_t **choose_son(uint32_t key, avl_node_t *node) {
	return (key < node->key) ? &node->left_son : &node->right_son;
}

/* returns 1 if node with given key was found otherwise 0
 * out will point to father's pointer to node with given key if such exists
 * if it doesn't it will point to father's pointer to last node visited by the 'find' operation
 */
static int avl_find_getaddr(avl_key_t key, avl_root_t *root, avl_node_t ***out) {
	avl_node_t **current_node, **current_son;
	current_node = current_son = &root->root_node;
	while (*current_son != NULL && (*current_node)->key != key) {
		current_node = current_son;
		current_son  = choose_son(key, *current_node);
	}
	*out = current_node;

	return *current_node != NULL && key == (*current_node)->key;
}

/* used exclusively inside avl_delete - DO NOT USE ELSEWHERE
 * handles changes of pointers between node with two sons and it's replacement
 * such deleted node is replaced with minimal node from it's right subtree
 * arguments are pointers to fathers' pointers to the nodes
 */
static void replace_node(avl_node_t **replaced, avl_node_t **replacement) {
	/* change father pointers in sons of replaced to point to replacement */
	if ((*replaced)->left_son != NULL)
		(*replaced)->left_son->father = *replacement;
	if ((*replaced)->right_son != NULL)
		(*replaced)->right_son->father = *replacement;

	/* change father pointer of replacement to that of replaced */
	(*replacement)->father = (*replaced)->father;

	avl_node_t *temp = (*replacement)->right_son;

	/* change son pointers of replacement to those of replaced */
	(*replacement)->left_son = (*replaced)->left_son;
	(*replacement)->right_son = (*replaced)->right_son;

	/* change father's pointer from replaced to replacement */
	*replaced = *replacement;

	/* exchange pointer to replacement for pointer to it's right son in it's original father */
	*replacement = temp;
}

/* returns number of non-null sons */
static int get_number_of_sons(avl_node_t *node) {
	return !!node->left_son + !!node->right_son;
}

/* returns pointer to father's pointer to minimal node in right subtree of root
 * expects right son of root to be non-null
 */
static avl_node_t **get_min_node(avl_node_t *root) {
	avl_node_t **min = &root->right_son;
	while ((*min)->left_son != NULL)
		min = &(*min)->left_son;
	return min;
}

/* AVL edge rotation
 *     |           |
 *     y           x
 *    / \         / \
 *   x   C  <->  A   y
 *  / \             / \
 * A   B           B   C
 * it is presumed that x and y are non-null
 * x,y represent nodes while A,B,C represent (possibly empty) subtrees
 */
static void rotate(avl_node_t **father, int left_to_right) {
	avl_node_t **ptr_to_son = left_to_right ? &(*father)->left_son : &(*father)->right_son;
	avl_node_t *son = *ptr_to_son;
	avl_node_t **Bnode = left_to_right ? &son->right_son : &son->left_son;

	/* make B son of y */
	if (*Bnode != NULL)
		(*Bnode)->father = *father;
	*ptr_to_son = *Bnode;

	/* move x to top */
	son->father = (*father)->father;
	*Bnode = *father;
	*father = son;
}

/* --- PUBLIC FUNCTIONS --------------------------------------- */

/* returns 1 if node with given key was found otherwise 0
 * out will point to node with given key if such exists
 * if it doesn't it will point to last node visited by the 'find' operation (use NULL to discard)
 */
int avl_find(avl_key_t key, avl_root_t *root, avl_node_t **out) {
	avl_node_t **out_local;
	int ret = avl_find_getaddr(key, root, &out_local);
	if (out != NULL)
		*out = *out_local;
	return ret;
}

/* if there is a node with key equal to that of new_node returns 1
 * otherwise inserts new_node and returns 0
 */
int avl_insert(avl_node_t *new_node, avl_root_t *root) {
	avl_node_t *father;
	if (avl_find(new_node->key, root, &father))
		return 1;

	new_node->father = father;
	*choose_son(new_node->key, father) = new_node;

	return 0;
}

/* if there is no node with given key, returns 1
 * otherwise deletes the appropriate node and returns 0
 * pointer to deleted node is stored in the deleted variable (use NULL to discard)
 */
int avl_delete(avl_key_t key, avl_root_t *root, avl_node_t **deleted) {
	avl_node_t **ptr_to_son;
	if (!avl_find_getaddr(key, root, &ptr_to_son))
		return 1;
	avl_node_t *node = *ptr_to_son;

	if (get_number_of_sons(node) < 2) {
		*ptr_to_son = (node->left_son != NULL) ? node->left_son : node->right_son;
	} else {
		avl_node_t **min = get_min_node(node);
		replace_node(ptr_to_son, min);
	}

	if (deleted != NULL)
		*deleted = node;

	return 0;
}

/* --- DEBUG -------------------------------------------------- */

void avl_enumerate(avl_node_t *root, int depth) {
	if(!root) {
		printf("%*c-\n", depth, ' ');
		return;
	}
	printf("%*c%d\n", depth, ' ', root->key);
	avl_enumerate(root->left_son, depth + 1);
	avl_enumerate(root->right_son, depth + 1);
}

int main() {
	avl_node_t one, two, three, four, five, six;
	avl_node_t *ptr_to_last;
	two   = (avl_node_t){2,&four,&one,NULL};
	four  = (avl_node_t){4,NULL,NULL,&two};
	one   = (avl_node_t){1,&five,&three,&two};
	five  = (avl_node_t){5,NULL,NULL,&one};
	three = (avl_node_t){3,NULL,NULL,&one};
	avl_root_t root = { .root_node = &two };

	avl_enumerate(root.root_node, 1);
	printf("\nrotated\n\n");
	rotate(&root.root_node, 0);
	avl_enumerate(root.root_node, 1);
}

#endif