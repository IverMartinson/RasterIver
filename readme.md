### How to Run?
To run the binary, it needs to be in the same folder as librasteriver.so (if you just cloned the repo then they both should be in the build folder). You also need SDL2 installed (libsdl2-dev)

### Notes
- When using the window's texture as a texture reference on an object that is being rendered to the window's texture, some parts of it won't be filled in becuase when those texels are accessed to draw the object, that part of the screen hasn't been rendered yet. 