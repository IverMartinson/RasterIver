# RasterIver, a Rendering Engine

<p align="center">
   <img src="https://mynameisthe.com/f/1764353133143-ezgif-692a8b4b3535e93c.gif" alt="gif of silly example scene"><br><i>17,700 polygon scene running at 100fps</i>
</p>

## Roadmap

### By End of December, 2025

 - [x] Basic texture support **`finished on November 26th, 2025`**
    - using custom image parser, PitMap
    - objects can have at least 1 texture
    - no animated textures yet
    - only BMP files
    - no bumpmap, normalmap, distortionmap, et cetera

 - [ ] Animated textures support
    - similar to Minecraft's where a texture repeats itself inside itself and you just offset the y coord based off the frame

### By End of January, 2026

 - [ ] Normal maps

 - [ ] occlusion culling

### By End of February, 2026

 - [ ] MTL support
 
 - [x] Triangle binning **`finished on November 28th, 2025`**

### By End of March, 2026

 - [ ] Custom TTF parser, SourParse, will be implemented
    - GPU font renderer

 - [ ] Animated textures will support GIF

### By End of April, 2026

 - [ ] Font renderer will use more efficent method
    - maybe polygons?
    - anything but the last implementation of ray-line intersection

### By End of June, 2026

 - [ ] PNG support
 - [ ] JPEG support