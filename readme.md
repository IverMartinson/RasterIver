
# RasterIver

World's Best Renderer, 2025

## How Does it Work?

### Building Blocks

- There are a couple fundimental elements of RasterIver: `actors`, `textures`, `materials`, `meshes`, and `scenes` (this isnt including internal ones like `vectors` and `faces`)
- They all use `pointers` to reference objects
- This allows multiple `actors`, for instance, to reference the same `material` or `mesh`
- Because most things' attributes are a reference, a `material` can have a `texture reference` to the `window`! (it's `frame buffer` is an `RI_texture`)

### Functions

- There are functions such as `RI_request_actors` which creates new `actors` and returns a `pointer` to the list of total `actors` that exist in the `RI context`
- Every function intended for the user will be prefixed with `RI_`

### Rasterizer Pipeline

- `RI_render` is called
- RasterIver checks if the `faces_to_render` array is allocated (this is an array that stores every valid, renderable `face` in the `scene` and is specific to each `scene`) and if it isn't, it loops through every `actor` in the `scene` and checks the total number of `faces`, then allocates 2x the needed memory (this is because sometimes `faces` need to be split into 2, meaning that you need another slot where you previously only needed one)
- `faces_to_render` is cleared
- RasterIver loops through every `actor` and calculates the transforms of positions, normals, and uvs for `face` in the `mesh` (it also performs shrinking and splitting culling here), checks if it's going to be in view, is a valid `face`, or has `should_render` set to `true`. If it is a valid `face`, it is added to the `faces_to_render` at the first open slot as a `RI_renderable_face` struct. The `AABB` of the `face` is also calculated here
- The `target texture`'s `image buffer` and the `z buffer` are cleared
- RasterIver loops through `faces_to_render` and then check every pixel in it's `AABB` and sees if it is inside the `face`. If it is, figure out what the color should be (does it have a `material`? A `texture`? Is it `wireframe`?)
- When `RI_tick` is called, it copies the `RI context`'s `image buffer` into the `SDL texture` and renders the frame onto the `window` (it also handles `SDL events`)

## How to Run?

To run the binary, it needs to be in the same folder as librasteriver.so (if you just cloned the repo then they both should be in the build folder). You also need SDL2 installed (libsdl2-dev)

## Todo List

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
- [ ] built in meshes (cubes, sphere, torus, capsule, cylinder)
- [ ] descriptions to variables and functions
- [ ] normal map shading
- [ ] bump maps
- [ ] normal maps
- [x] memory tracker