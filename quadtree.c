#include <stdlib.h>

#include "quadtree.h"

quadtree_node* quadtree_node_new(void)
{
	quadtree_node* node = malloc(sizeof(quadtree_node));

	node->parent = NULL;

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

quadtree_node* quadtree_node_new_c(double r, double g, double b, double a)
{
	quadtree_node* node = quadtree_node_new();

	node->r = r;
	node->g = g;
	node->b = b;
	node->a = a;

	return node;
}

void quadtree_node_kill(quadtree_node* node)
{
	if (node == NULL) return;

	for (size_t i = 0; i < 4; i++)
	{
		quadtree_node_kill(node->children[i]);
	}

	free(node);
}

quadtree_node* quadtree_index_force(quadtree_node* node, unsigned int i)
{
	quadtree_node* child = node->children[i];

	if (child == NULL)
	{
		child = quadtree_node_new();
		child->parent = node;

		node->children[i] = child;
	}

	return child;
}

void quadtree_node_update(quadtree_node* node)
{
	size_t totalchildren = 0;

	double r = 0;
	double g = 0;
	double b = 0;
	double a = 0;

	size_t n = 0;

	for (size_t i = 0; i < 4; i++)
	{
		quadtree_node* child = node->children[i];

		if (child != NULL)
		{
			// count the child node if it's on
			totalchildren++;

			// and all of its children
			totalchildren += child->totalchildren;

			if (child->on)
			{
				// average in its color
				r += child->r;
				g += child->g;
				b += child->b;
				a += child->a;
				n++;
			}
		}
	}

	node->totalchildren = totalchildren;

	// if this node has any children, set its color
	//  to the average of its childrens' colors
	if (n)
	{
		node->r = r / n;
		node->g = g / n;
		node->b = b / n;
		node->a = a / n;

		node->on = NODE_CHILDON;
	}
	else if (node->on == NODE_CHILDON)
	{
		node->on = NODE_OFF;
	}

	/*
	if (node->parent != NULL)
	{
		quadtree_node_update(node->parent);
	}
	*/
}

size_t nprune = 0;

int quadtree_prune(quadtree_node* node)
{
	if (node == NULL) return 0;

	for (size_t i = 0; i < 4; i++)
	{
		quadtree_node* child = node->children[i];
		if (quadtree_prune(child))
		{
			quadtree_node_kill(child);
			node->children[i] = NULL;

			nprune++;
		}
	}

	quadtree_node_update(node);

	return node->totalchildren == 0 && node->on == NODE_OFF;
}
