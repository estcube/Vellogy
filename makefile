CFLAGS = -Wall -pedantic-errors -ansi -Wextra 
LDLIBS = -lm -g

Prog : main.o stack.o
	gcc -o Prog main.o stack.o
