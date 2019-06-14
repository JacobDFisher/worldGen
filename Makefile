LDLIBS = -lm `pkg-config --static --libs glfw3 glew`
CFLAGS += -g

all:
	worldGen townGen

worldGen: worldGen.c

townGen: townGen.c

didneyWorl: didneyWorl.c

roundWorld: roundWorld.c

clean:
	rm -f didneyWorl
