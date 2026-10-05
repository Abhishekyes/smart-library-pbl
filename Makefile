# Build:  make        Run:  ./library   (run from this folder so data/ is found)
CC  = gcc
CXX = g++
OBJS = src/list.o src/search.o src/oop.o src/main.o

library: $(OBJS)
	$(CXX) -o library $(OBJS)

src/%.o: src/%.c src/library.h
	$(CC) -Wall -c $< -o $@

src/%.o: src/%.cpp src/library.h src/oop.h
	$(CXX) -Wall -c $< -o $@

clean:
	rm -f src/*.o library
