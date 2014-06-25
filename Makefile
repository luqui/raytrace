all:
	g++ -Wall -O2 -o main main.cpp -I. `sdl-config --cflags --libs` -framework OpenGL -lSDL_image

prof:
	g++ -Wall -o main main.cpp -I. -g -pg `sdl-config --cflags --libs` -framework OpenGL -lSDL_Image
