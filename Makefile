all:
	g++ -O2 -o main main.cpp -I. `sdl-config --cflags --libs`
