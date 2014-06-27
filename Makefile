all:
	g++ -Wall -Wno-unknown-pragmas -O2 -o main main.cpp -I. `sdl-config --cflags --libs` -framework OpenGL -lSDL_image

prof:
	g++ -Wall -Wno-unknown-pragmas -o main main.cpp -I. -g -pg `sdl-config --cflags --libs` -framework OpenGL -lSDL_Image
