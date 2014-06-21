all:
	g++ -O2 -o main main.cpp -I. `sdl-config --cflags --libs`

prof:
	g++ -O2 -o main main.cpp -I. -g -pg `sdl-config --cflags --libs`
