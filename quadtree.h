#ifndef QUADTREE_H
#define QUADTREE_H

typedef struct quadtree_node
{
	struct quadtree_node* children[4];

	size_t totalchildren;

	double r;
	double g;
	double b;
	double a;


} quadtree_node;


// constructors/destructors for quadtree_node
quadtree_node* quadtree_node_new(void);
quadtree_node* quadtree_node_new_c(double r, double g, double b, double a);

void quadtree_node_kill(quadtree_node* node);

// methods for operating on the tree
quadtree_node* quadtree_node_get(quadtree_node** nodeptr);

void quadtree_node_update(quadtree_node** nodeptr, quadtree_node* node);


#endif
