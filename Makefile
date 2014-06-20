all:
	g++ -O2 -o main main.cpp `sdl-config --cflags --libs`
