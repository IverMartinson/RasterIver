# RasterIver, a Rendering Engine
<p align="center">
  <img src="https://mynameisthe.com/f/1749237242096-rasteriver_example.gif" alt="animated" >
</p>

#### RasterIver?
Rasterizer + Iver = RasterIver

## Features
(todo: write features)

## RI_FLAG List
- `RI_FLAG_DEBUG` Turns debugging on or off
- `RI_FLAG_DEBUG_LEVEL` Indepth-ness of debugging 
- `RI_FLAG_SHOW_BUFFER` The "buffer" to render
- `RI_FLAG_SHOW_FPS` Shows the FPS
- `RI_FLAG_DEBUG_FPS` Prints FPS in the console
- `RI_FLAG_CLEAN_POLYGONS` Clear the polygon array after RI_RequestPolyons is called
- `RI_FLAG_POPULATE_POLYGONS` Sets random values in the polygon array
- `RI_FLAG_BE_MASTER_RENDERER` Whether you want to use objects or to have direct access to the polygon array
- `RI_FLAG_SHOW_FRAME` Shows the current frame #
- `RI_FLAG_DEBUG_FRAME` Prints the current frame # in the console
- `RI_FLAG_SHOW_INFO` Shows object info
- `RI_FLAG_DEBUG_TICK` Turns debugging on or off for anything that would fill up the console when constantly ticking
- `RI_FLAG_USE_CPU` Use the CPU over the GPU
- `RI_FLAG_HANDLE_SDL_EVENTS` Whether RasterIver should handle SDL events

## Requirements
#### To Run
- SDL2
- SDL2 TTF
- OpenCL Runtimes
#### To Build
- SDL2
- SDL2 TTF
- OpenCL Developer Libraries
- ROCm if using AMD
- CUDA if using NVIDIA


# Todo
- [x] make todo list
- [ ] function to request a mesh. I.E., request a number of polygons/verticies (maybe from presets? planes, cubes, idk) and be able to edit them on the fly. Useful for dynamic terrain or something like that
- [x] texture support
- [x] fix interpolation issue with UV maps (and probably normals and Z values too)
- [ ] fix horribly named variables in request object functions
- [ ] include rasteriver.h in the kernels for better code updatability
- [ ] make kernels not be in strings (read the file at compile time and put it inside rasteriver.c as a string? put it in a header like how it is now?)
- [x] change all iterator variables in for loops to have "i_" before it's variable name
- [x] add descriptions to RI functions
- [x] optimize object memory usage by not loading object data multiple times when the same object is requested
- [x] add checks for invalid/nonexistant files (files like object files & texture files)
- [x] perspective
- [x] euler rotation
- [ ] quaternion rotation
- [ ] sheer transform (and other fancy ones?)
- [ ] simple lighting using normals
- [ ] complex lighting using rays for shadows and stuff
- [ ] polygon clipping by subdividing (do we need this? I think the logic rn is fine without it)
- [x] flag for using CPU instead of GPU
- [ ] actually acurate FPS cap
- [ ] make an option for multiple cameras, or defining a camera. I want to be able to write it to a "texture" in menory and have objects use it as their texture
- [ ] add shaders. Maybe have shader kernels? I have no idea how you would implement that but it sounds cool
- [x] make the returned array from RI_RequestObjects easier to write to I.E., not a 1D array. Maybe have it so you could do objects[obj #].x_position or something like that. I wonder if I could implement functions with that, too, so you could do objects.get_first_element()
- [ ] object higherarchies (maybe not. This is moving towards game engine territory. Once I get this into a working release, I'll start working on a game engine)
- [ ] deltaTime variable or function. (function that returns pointer? function that returns the last deltatime value?)
- [x] make a function to set a custom debug prefix
- [ ] add multiple texture support for objects
- [ ] add support for normal maps
- [ ] add support for bump/height/displacement maps
- [ ] add support for transparent textures
- [ ] allow objects to have no texture
- [ ] make checks for objects trying to have a texture, but no UV coords (generate them?)
- [x] FOV
- [x] add materials
- [x] make another kernel that calculated transforms & perspective before rasterizing
- [ ] add ability to request objects multiple times 
- [ ] give objects IDs or some way to track them so that you can remove them dynamically
- [ ] make CUDA version because OpenCL is slow with NVIDIA
- [x] [this wasn't actually a problem. I had -999999999 for r_w but switched how the inputs are. It was rotatiing things -99999999 rad and looked like it was off] fix center of perspective being in the wrong spot
- [ ] make function for world scale/master scale (scales everything. from origin? from object origins?)
- [x] I think calculations for debugging memory usage are outdated/wrong
- [ ] use correct types for stuff (like size_t)
- [ ] add target resolution
- [ ] find a faster way to draw pixels to the window
- [ ] replace "RI_RequestObjects" with 2 functions, RI_CreateObject (which returns a pointer to the object) and RI_LoadObjectFile (which takes in an object file and a pointer). It'll be tough making the object data arrays basically dynamic, but it shouldn't be too hard. Maybe I'll make a termination sequence after each object's data points. Like [x, y, z..... NULL, 0, INFINITY, x, y, z......]. If I know the object I want to delete, I know the start and stopping points of all it's data points. All I have to do is cut them out. Is there a way to squish it back together though? Maybe I could shift all the other data points down, but if there is a substantial amount of them after the cut one, it could take forever.
- [ ] antialiasing
- [ ] fix gaps in thin lines
- [ ] add "transformed" before verticies and normals memory buffers in main kernel to make it less confusing
- [ ] add overdraw buffer