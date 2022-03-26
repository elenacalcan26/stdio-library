CC = gcc

build: libso_stdio.so

libso_stdio.so: so_stdio.o
	$(CC) -shared $^ -o $@

so_stdio.o: so_stdio.c
	$(CC) -Wall -Wextra -fPIC -c $^ -o $@

clean:
	rm *.o