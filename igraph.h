void quadtree_search(quadtree_node* node, double xl, double xh, double yl, double yh);
void quadtree_render(quadtree_node* node, double xl, double xh, double yl, double yh);

void* calc(void* threadid);

void setview(double _xmid, double _ymid, double _zoom);

int main(int argc, char** argv);
