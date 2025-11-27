# RasterIver, a Rendering Engine

## Roadmap

### By End of December, 2025

 - [x] Basic texture support **`finished on November 26, 2025`**
    - using custom image parser, PitMap
    - objects can have at least 1 texture
    - no animated textures yet
    - only BMP files
    - no bumpmap, normalmap, distortionmap, et cetera

 - [ ] Animated textures support
    - similar to Minecraft's where a texture repeats itself inside itself and you just offset the y coord based off the frame

### By End of January, 2026

 - [ ] Normal maps

### By End of February, 2026
 - [ ] MTL support

### By End of March, 2026

 - [ ] Rasterizer will use some sort of tree system for performance
    - quad tree, K-D tree, et cetera

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