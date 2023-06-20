test: test.c libvec.h
	clang -g -o $@ test.c 
	./test