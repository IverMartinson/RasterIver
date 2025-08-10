### How to Run?
To run the binary, it needs to be in the same folder as librasteriver.so (if you just cloned the repo then they both should be in the build folder). You also need SDL2 installed (libsdl2-dev)

### Todo
[ ] checks for missing references
[ ] checks for missing files
[ ] debugging
[ ] support for 4 side+ polygons in the OBJ parser
[ ] support for negative UV and normals in the OBJ parser
[ ] .MTL support
[ ] save scenes as templates? (files)
[ ] fix camera rotation code
[ ] fix OBJ parser issue with larger models? (idk, the teapot and gordon both dont work and I don't know why)
[x] allow for aspect ratios other than 1:1
[ ] fix strange visual issue that happens when (speculation) a triangle gets culled and split very close to the clipped point
[ ] port CPU renderer to GPU