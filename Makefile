all:
	g++ -Wall -O2 -o main main.cpp -I. `sdl-config --cflags --libs` -framework OpenGL

prof:
	g++ -Wall -o main main.cpp -I. -g -pg `sdl-config --cflags --libs` -framework OpenGL
