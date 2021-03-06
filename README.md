# igraph

Implicit equation grapher in C, using a 2d binary search to find points on the graph and a quadtree to store them.

Works well, but is still incomplete due to several notorious problems such as the fact that equation to be graphed is currently hardcoded in `func.c`.

Can be easily abused as a fairly efficient fractal viewer by giving it a function that returns `-1` for points outside of the fractal and `1` for points inside the fractal.  An example of doing this for the Mandelbrot set may be found in `func.c`.

This software is licensed under GPLv3.  See the file `LICENSE` for details.

# Compiling / Running

`igraph` requires GLFW and pthreads.  It probably only runs on Unix systems right now, but could probably be easily made to work on Windows.

First, install [GLFW](http://www.glfw.org/).  Then, just run `make && ./igraph`.  Until dynamic graph function loading is implemented, you'll generally want to run these two together, since the only way to edit the function currently is by editing the function in `func.c` and recompiling.

I'm currently not using pkg-config or anything, so it's possible you'll need to edit the Makefile to help it file GLFW.  Sorry about this, I'll try to fix it soon.

# Graphing Your Own Equation

Your equation needs to be of the form `h(x, y) = 0`.  If it isn't, and is instead of the form `f(x, y) = g(x, y)`, you can obtain `h(x, y) = f(x, y) - g(x, y)`.  Make sure that `h(x, y)` actually crosses zero and changes sign.  For example, if you wanted to solve `y = x` and specified `h(x, y) = (x - y)^2` instead of `h(x, y) = x - y`, `igraph` wouldn't display anything, since `h(x, y) = (x - y)^2` never crosses zero but merely touches it and then stays on the same side.

The graph function is currently hardcoded.  Open up `func.c`, and edit the `double func(double x, double y)` function to return your `h(x, y)`.  Obviously, it needs to be in C syntax.

# Principle of Operation

`igraph` stores all of its data in a [quadtree](https://en.wikipedia.org/wiki/Quadtree).  `igraph`'s quadtree data structure makes no technical distinction between branch and leaf nodes - all nodes have all the properties of branch and leaf nodes.  Nodes with children are referred to as branch nodes and nodes without children are reffered to as leaf nodes in this discussion.  Branch nodes can be treated as leaf nodes - they still carry a color, which is the average of the colors of all of their children.

`igraph` uses two threads: a render thread and a search thread.  The render thread iterates over the quadtree and draws its contents.  It skips all nodes entirely outside the viewing window, and treats branch nodes smaller than a pixel as if they were leaf nodes.  The search thread also iterates over the quadtree, skipping all nodes entirely outside the viewing window.  It continues iterating until it reaches a node with side length less than `mindepth` pixels.  At that point, it evaluates `func(x,y)` at all four corners of the node and looks at the signs of the answers.  If any of the signs differ, the function must cross zero at some point within that node, so it keeps iterating until the node size is down to `maxdepth` pixels.  When it is time to stop iterating, if it stopped because it had reached maximum depth, it stores the fact that the graph intersects the current node in the tree.

For a visualization of the quadtree, open `igraph.c`, find the only occurrence of `#if 0`, replace it with `#if 1`, recompile, and run.  It runs significantly slower with this enabled on my computer.

# TODO

* The graph function is currently hardcoded in func.c - it needs to be made dynamic somehow
  * It should be compiled for speed
  * Candidates I have in mind: `tcc`, `cc` with `dlopen`, or `luajit`
* Zoom range is currently limited to `2^20` to around `10^-16`.  The upper bound is because that's what I set it to, and the lower bound has to do with the limits of `double` precision.
* Cannot tell the difference between a point that is actually a solution or a point that is a discontinuity that crosses 0.  A simple threshold might work, or a numerical derivative or something.  It should color discontinuities a different color, such as blue.
* The fancy fade-in grid code is slightly broken - you can tell when it shifts grid sizes, and there's funky checkerboard aliasing at some zoom levels.
* Smarter calculation order - bottom-left first isn't too great
  * maybe do what XaoS does - focus on the cursor position if zooming in and on the screen edge if zooming out?
  * maybe do a shallower search while moving, and then go to full depth when not moving or when moving slowly
* XaoS-style zoom - left-click to zoom in, right-click to zoom out
* Drop data from the tree to keep it from getting too big
  * This isn't a problem unless you repeatedly zoom in and out in different places with a complicated graph (such as a fractal) for a long time until you run out of RAM.
* Improve branch node color averaging - it looks bad when you zoom out, especially with `SHOW_SIGN` on


This doesn't segfault on my machine with no mutexes and `-O2` or `-O3`.  Please file a bug if it segfaults for you.
