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
- [ ] polygon clipping by subdividing
- [x] flag for using CPU instead of GPU
- [ ] actually acurate FPS cap
- [ ] make an option for multiple cameras, or defining a camera. I want to be able to write it to a "texture" in menory and have objects use it as their texture
- [ ] add shaders. Maybe have shader kernels? I have no idea how you would implement that but it sounds cool
- [ ] make the returned array from RI_RequestObjects easier to write to I.E., not a 1D array. Maybe have it so you could do objects[obj #].x_position or something like that. I wonder if I could implement functions with that, too, so you could do objects.get_first_element()
- [ ] object higherarchies (maybe not. This is moving towards game engine territory. Once I get this into a working release, I'll start working on a game engine)
- [ ] deltaTime variable or function. (function that returns pointer? function that returns the last deltatime value?)
- [ ] make a function to set a custom debug prefix
- [ ] add multiple texture support for objects
- [ ] add support for normal maps
- [ ] add support for bump/height/displacement maps
- [ ] add support for transparent textures
- [ ] allow objects to have no texture
- [ ] make checks for objects trying to have a texture, but no UV coords (generate them?)
- [x] FOV
- [ ] add materials