### How to Run?
To run the binary, it needs to be in the same folder as librasteriver.so (if you just cloned the repo then they both should be in the build folder). You also need SDL2 installed (libsdl2-dev)

### Todo
- [ ] checks for missing references
- [ ] checks for missing files
- [ ] debugging
- [ ] 4 side+ polygons in the OBJ parser
- [ ] negative UV and normals in the OBJ parser
- [ ] .MTL support (?)
- [ ] save scenes as templates? (files)
- [ ] fix camera rotation code
- [x] OBJ parser issue with larger models? (idk, the teapot and gordon both dont work and I don't know why) [NOTE: it was because large models' faces randomly had should_render turned off. No clue why the small ones didn't but whatever]
- [x] aspect ratios other than 1:1
- [x] fix strange visual issue that happens when (speculation) a triangle gets culled and split very close to the clipped point [NOTE: it was a floating point precision issue. I changed all the floats to doubles and it's gone]
- [ ] port CPU renderer to GPU
- [ ] seams in objects
- [ ] other object filetypes
- [ ] flip texture function
- [ ] multiple texture filetypes
- [ ] texture resizing when passing in a different resolution than the texture is
- [ ] add built in meshes (cubes, sphere, torus, capsule, cylinder)