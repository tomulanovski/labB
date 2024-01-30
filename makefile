all: virdec

virdec: virdec.o
	gcc -Wall -g -m32 -o virdec virdec.o

virdec.o: virdec.c
	gcc -Wall -g -m32 -c virdec.c

.PHONY: clean

clean:
	rm *.o virdec