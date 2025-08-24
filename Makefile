COMPILER=gcc
FLAGS_ALL=-fsanitize=address -g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter -Wno-sequence-point
FLAGS_EXAMPLE=-Lcompiled_libs/ -lrasteriver -lsourparse -Wl,-rpath=build/ -lm -lSDL2
FLAGS_LIB=-D CL_TARGET_OPENCL_VERSION=120 -fPIC -shared -lc -lSDL2 -lm -lOpenCL

main.bin: clean rasteriver.so
	cp compiled_libs/librasteriver.so build/librasteriver.so
	cp compiled_libs/libsourparse.so build/libsourparse.so
	$(COMPILER) $(FLAGS_ALL) src/launch_program/main.c -o build/main.bin $(FLAGS_EXAMPLE) 

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/library/rasteriver.c -o compiled_libs/librasteriver.so $(FLAGS_LIB) 

clean:
	-rm build/*
