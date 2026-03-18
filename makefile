all:
	c++ `pkg-config --cflags glm` -std=c++23 src/raytracer.cpp -o raytracer

clean:
	rm -f raytracer
