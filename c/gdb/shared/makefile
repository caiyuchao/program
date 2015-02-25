all: main libcrash.so core

main: main.o libcrash.so
	gcc -g -o $@ $< -L. -lcrash

main.debug: main
	strip main.out --only-keep-debug -o  main.debug

#main.release: main
#	strip --strip-all $< -o $@

core: main
	ulimit -c unlimited; LD_LIBRARY_PATH=. ./main

libcrash.so: crash.o
	gcc -shared -o $@ $<

clean:
	rm -f main core main.out main.debug *.o *.so
