CC=gcc

mysh: mysh.c
	$(CC) -o mysh -Wall -Werror -O3 -g mysh.c

clean:
	rm -f mysh
