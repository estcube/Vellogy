CFLAGS = -Wall -pedantic-errors -Wextra 
LDLIBS = -lm -lrt -g

Prog : main.o stack.o
	gcc -o Prog main.o stack.o $(CFLAGS) $(LDLIBS)


