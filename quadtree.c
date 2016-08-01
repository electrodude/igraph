#include <stdlib.h>

#include "quadtree.h"

quadtree_node *quadtree_node_new(void)
{
	quadtree_node *node = malloc(sizeof(quadtree_node));

	node->totalchildren = 0;

	for (size_t i = 0; i < 4; i++)
	{
		node->children[i] = NULL;
	}

	node->r = 0.0;
	node->g = 0.0;
	node->b = 0.0;
	node->a = 0.0;


	return node;
}

quadtree_node *quadtree_node_new_c(double r, double g, double b, double a)
{
	quadtree_node *node = quadtree_node_new();

	node->r = r;
	node->g = g;
	node->b = b;
	node->a = a;

	return node;
}

void quadtree_node_kill(quadtree_node *node)
{
	if (node == NULL) return;

	// recursively free all children
	for (size_t i = 0; i < 4; i++)
	{
		quadtree_node *child = node->children[i];
		//node->children[i] = NULL; // for the render thread's sake

		quadtree_node_kill(child);
	}

	// free this node's memory
	free(node);
}


quadtree_node *quadtree_node_get(quadtree_node **nodeptr)
{
	// If there's nothing there
	if (*nodeptr == NULL)
	{
		// then return a new one
		return quadtree_node_new();
		// The new one is inserted into the tree by quadtree_node_update,
		//  or a simple *nodeptr = node if it's a leaf node
	}

	// If there was something there, return it
	return *nodeptr;
}

void quadtree_node_update(quadtree_node **nodeptr, quadtree_node *node)
{
	if (nodeptr == NULL) return;

	if (node == NULL) return;

	size_t totalchildren = 0;

	double r = 0;
	double g = 0;
	double b = 0;
	double a = 0;

	size_t children = 0; // immediate children

	for (size_t i = 0; i < 4; i++)
	{
		quadtree_node *child = node->children[i];

		if (child != NULL)
		{
			// count the child node
			totalchildren++;

			// and all of its children
			totalchildren += child->totalchildren;

			// make r, g, b, a the max of all of its childrens' values
			if (child->r > r) r = child->r;
			if (child->g > g) g = child->g;
			if (child->b > b) b = child->b;
			if (child->a > a) a = child->a;

			children++;
		}
	}

	node->totalchildren = totalchildren;

	// If this node has any children
	if (children)
	{
		// then set each component of its color to the maximum value
		//  of that component that any of its children had
		node->r = r;
		node->g = g;
		node->b = b;
		node->a = a;

		// This node has children and is here to stay,
		//  so store it in the tree
		*nodeptr = node;
	}
	// Otherwise, if this node has no children and wasn't put into the tree yet
	else if (*nodeptr == NULL)
	{
		// then don't bother storing it; delete it
		quadtree_node_kill(node);
	}

	// We don't need to update this node's parent since
	//  all callers of this function do it for us
}
