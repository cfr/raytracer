all:
	c++ `pkg-config --cflags glm` -Isrc -std=c++23 src/raytracer.cpp -o raytracer

clean:
	rm -f raytracer
