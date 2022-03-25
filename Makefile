CC = gcc

build: libso_stdio.so

libso_stdio.so: so_stdio.o
	$(CC) -shared $^ -o $@

so_stdio.0: so_stdio.c
	$(CC) -Wall -Wextra -fPIC -c $^ -o $@

clean:
	rm *.o