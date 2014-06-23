all:
	g++ -O2 -o main main.cpp -I. `sdl-config --cflags --libs` -lGL

prof:
	g++ -o main main.cpp -I. -g -pg `sdl-config --cflags --libs` -lGL
