LFLAGS = -lm
CFLAGS = -Wall -std=c99 

all: plot

plot: plot.o tinyexpr.o
	gcc plot.o tinyexpr.o -o plot `sdl2-config --cflags --libs` $(LFLAGS)

plot.o: plot.c tinyexpr/tinyexpr.h
	gcc plot.c -c $(CFLAGS)

tinyexpr.o: tinyexpr/tinyexpr.c tinyexpr/tinyexpr.h
	gcc tinyexpr/tinyexpr.c -c $(CFLAGS)

clean: 
	rm *.o plot
