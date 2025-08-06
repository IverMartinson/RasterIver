COMPILER=gcc
FLAGS_ALL=-g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter
FLAGS_EXAMPLE=-Lbuild/ -lrasteriver -Wl,-rpath=build/ -lm -lSDL2
FLAGS_LIB=-D CL_TARGET_OPENCL_VERSION=120 -fPIC -shared -lc -lSDL2 -lSDL2_ttf -lm -lOpenCL

main.bin: rasteriver.so
	$(COMPILER) $(FLAGS_ALL) src/launch_program/main.c -o build/main.bin $(FLAGS_EXAMPLE) 

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/library/rasteriver.c -o build/librasteriver.so $(FLAGS_LIB) 

clean:
	rm build/*
