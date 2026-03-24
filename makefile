all:
	c++ -Wall -Wextra -g `pkg-config --cflags glm` -Isrc -std=c++23 src/raytracer.cpp -o raytracer

trace:
	@for f in scenes/*; do \
		date "+%H:%M:%S"; \
		echo $$f; \
		./raytracer $$f; \
	done

clean:
	rm -f raytracer
