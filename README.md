# Chaos
An exercise in plotting fractal equations. Displays either Sierpinski Triangle, Sierpinski Carpet, Henon Attractor or Julia Set plotted by a random algorithm. A new pattern is chosen on the hour. Pebble Time version adds a cool color shifting algorithm.

This version uses fixed point math provided by the math-sll.h library instead of floating point. Since the pebble doesn't have a dedicated FPU, this results in faster fractal generation and smaller footprint in memory.

2.1 - To save battery, the plotting routines are no longer called after 15 minutes. However, the colors will still change on the minute. Also, coloring routines for the Henon Attractor and Sierpinski Carpet have been changed to be more visually pleasing.

2.0 - Better positioning algorithm for Julia and Mandlebrot. Program checks to ensure position doesn't end up mostly blank or interior of Mandelbrot "lake". Improved visibility of Julia and Mandelbrot on B&W Pebbles.
