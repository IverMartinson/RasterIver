COMPILER=gcc
FLAGS_ALL=-g -Wall -Wextra
FLAGS_EXAMPLE=-Lbuilds/final\ binaries -lrasteriver -Wl,-rpath=builds/final\ binaries/
FLAGS_LIB=-D CL_TARGET_OPENCL_VERSION=120 -fPIC -shared -lc -lSDL2 -lm -lOpenCL

example.bin: rasteriver.so
	$(COMPILER) $(FLAGS_ALL) src/test\ programs/example.c -o builds/final\ binaries/example.bin $(FLAGS_EXAMPLE) 

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/RasterIver/source\ code/rasteriver.c -o builds/final\ binaries/librasteriver.so $(FLAGS_LIB) 

clean:
	rm builds/final\ binaries/*