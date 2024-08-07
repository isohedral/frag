# frag
A simple 2D fragment shader viewer, inspired by Shadertoy (www.shadertoy.com)

This tool was designed to support one small feature over and above Shadertoy: the ability to navigate a virtual 2D plane interactively, using the mouse.  (This is possible with Shadertoy as well, but it's inconvenient.)  I used it to explore a number of algorithms for drawing repeating patterns implicitly (i.e., draw a tiling by sampling positions in the plane rather than pushing polygons through the graphics pipeline).

# Building (Mac)
* brew install glm --build-from-source
* brew install glew --build-from-source
* git submodule update
* make

# Launching:

Straightforward, just run a fragment shader:
```sh
./frag something.frag
```
Run a fragment shader that reads from a texture:
```sh
./frag -tex Examples/tile/tile_aqua.png Examples/lizards1.frag
```
Run a fragment shader that writes in multiple passes to an FBO and then reads from the FBO to write to the screen.
```
./frag fbo_1.frag fbo_2.frag ... fbo_n.frag compose.frag
```

# The shader:

The program wraps your shader code inside boilerplate containing a
few additional declarations via uniforms.  The main ones are simple:

 * `wpos`: a `vec2d` with the 2D world coordinates of the point to sample
 * `col`: a `vec4` to hold the output colour.

Your shader should be a function called `main()` that consumes no
arguments and writes to `col`.

The only other things defined are:
 * `tex`: if an FBO is used, it'll be here.
 * `tiletex`: if a texture is loaded via the commandline, it will be available in a sampler called tiletex.
 * `resolution`: an `ivec2D` telling you the width and height of the framebuffer.

The program depends on GLFW, STB_IMAGE, and GLM.  It's not particularly
well written, but it could easily be cleaned up and extended.
