# RasterIver, a GPU Accelerated Rendering Engine
<p align="center">
  <img src="https://mynameisthe.com/f/1749237242096-rasteriver_example.gif" alt="animated" >
</p>

#### RasterIver?
Rasterizer + Iver = RasterIver
### Requirements
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
- [ ] fix interpolation issue with UV maps (and probably normals and Z values too)
- [ ] fix horribly named variables in request object functions
- [ ] include rasteriver.h in the kernels for better code updatability
- [ ] make kernels not be in strings (read the file at compile time and put it inside rasteriver.c as a string? put it in a header like how it is now?)
- [ ] change all iterator variables in for loops to have "i_" before it's variable name
- [ ] add descriptions to RI functions
- [ ] make a function call stack to review where errors happen or just debug stuff
- [ ] optimize object memory usage by not loading object data multiple times when the same object is requested
- [ ] add checks for invalid/nonexistant files (files like object files & texture files)
- [ ] perspective
- [x] euler rotation
- [ ] quaternion rotation
- [ ] sheer transform (and other fancy ones?)
- [ ] simple lighting using normals
- [ ] complex lighting using rays for shadows and stuff
- [ ] polygon clipping by subdividing
- [x] flag for using CPU instead of GPU
- [ ] actually acurate FPS cap
- [ ] make a function to return a pointer to the last SDL_event (or copy it doesnt really matter)
- [ ] make an option for multiple cameras, or defining a camera. I want to be able to write it to a "texture" in menory and have objects use it as their texture
- [ ] add materials/shaders that define unlit, lit, textures, and whatever else. Maybe have shader kernels? I have no idea how you would implement that but it sounds cool