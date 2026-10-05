# Build:  make        Run:  ./library      Check everything:  make test
# (run from this folder so data/ is found)
library: src/library.c src/main.cpp src/library.h
	gcc -Wall -Wextra -c src/library.c -o src/library.o
	g++ -Wall -Wextra -c src/main.cpp -o src/main.o
	g++ -o library src/library.o src/main.o

test: library
	@bash run_tests.sh | tail -4

clean:
	rm -f src/*.o library
