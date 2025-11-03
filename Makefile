COMPILER=gcc
FLAGS_ALL=-g -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter
FLAGS_EXAMPLE=-Lbuilds/ -lrasteriver -Wl,-rpath=builds/ -lm -lSDL2
FLAGS_LIB=-fPIC -shared -lc -lSDL2 -lm -lOpenCL

main.bin: rasteriver.so
	$(COMPILER) $(FLAGS_ALL) src/launch\ program/main.c -o builds/main.bin $(FLAGS_EXAMPLE) 

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/main/main.c -o builds/librasteriver.so $(FLAGS_LIB) 

clean:
	rm builds/*
