test: test.c libvec.h
	clang --std=gnu11 -Wall -Werror -Wextra -pedantic -O2 -o $@ test.c 
	./test