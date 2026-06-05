COMPILER=gcc
FLAGS_ALL=-g -O3 -Wall -Wextra -Wno-unused-variable -Wno-unused-parameter 
FLAGS_EXAMPLE=-Lbuilds/ -lrasteriver -lpitmap -lkiyomitree -Wl,-rpath=builds/ -lm -lSDL2
FLAGS_LIB=-fPIC -shared -lc -lSDL2 -lm 
FLAGS_ASAN=-fsanitize=address -fno-omit-frame-pointer

main.bin: rasteriver.so pitmap.so kiyomitree.so
	$(COMPILER) $(FLAGS_ALL) src/launch\ program/main.c -o builds/main.bin $(FLAGS_EXAMPLE) 

asan: rasteriver.so pitmap.so kiyomitree.so
	$(COMPILER) $(FLAGS_ALL) $(FLAGS_ASAN) src/launch\ program/main.c -o builds/main.bin $(FLAGS_EXAMPLE)

run: main.bin
	./run

dbg: main.bin
	./dbg

rasteriver.so:
	$(COMPILER) $(FLAGS_ALL) src/main/main.c -o builds/librasteriver.so $(FLAGS_LIB) 

pitmap.so:
	cp src/dependencies/libpitmap.so builds/

kiyomitree.so:
	cp src/dependencies/libkiyomitree.so builds/

clean:
	rm builds/*
