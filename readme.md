# RasterIver, a GPU Accelerated Rendering Engine
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
- [x] add todo list
- [ ] add function to request a mesh. I.E., request a number of polygons/verticies (maybe from presets? planes, cubes, idk) and be able to edit them on the fly. Useful for dynamic terrain or something like that
- [ ] add texture support
- [ ] fix horribly named variables in request object functions
- [ ] include rasteriver.h in the kernels for better code updatability
- [ ] make kernels not be in strings (read the file at compile time and put it inside rasteriver.c as a string? put it in a header like how it is now?)