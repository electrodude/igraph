#ifndef QUADTREE_H
#define QUADTREE_H

typedef enum node_state
{
	NODE_OFF = 0,
	NODE_CHILDON = 1,
	NODE_ON = 2,
} node_state;

typedef struct quadtree_node
{
	struct quadtree_node* parent;
	struct quadtree_node* children[4];

	size_t totalchildren;

	double r;
	double g;
	double b;
	double a;

	node_state on;
} quadtree_node;



quadtree_node* quadtree_node_new(void);
quadtree_node* quadtree_node_new_c(double r, double g, double b, double a);

quadtree_node* quadtree_index_force(quadtree_node* node, unsigned int i);

void quadtree_node_update(quadtree_node* node);

extern size_t nprune;

int quadtree_prune(quadtree_node* node);


#endif
