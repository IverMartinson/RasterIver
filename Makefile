COMPILER=gcc
FLAGS_ALL=-g -Wall -Wextra
FLAGS_EXAMPLE=-Lbuilds/final\ binaries -lrasteriver -Wl,-rpath=builds/final\ binaries/ -lm
FLAGS_LIB=-D CL_TARGET_OPENCL_VERSION=120 -fPIC -shared -lc -lSDL2 -lSDL2_ttf -lm -lOpenCL

main.bin: rasteriver.so
	$(COMPILER) $(FLAGS_ALL) src/launch\ program/main.c -o builds/final\ binaries/main.bin $(FLAGS_EXAMPLE) 

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/RasterIver/source\ code/rasteriver.c -o builds/final\ binaries/librasteriver.so $(FLAGS_LIB) 

clean:
	rm builds/final\ binaries/*
