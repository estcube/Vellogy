CFLAGS = -Wall -pedantic-errors -Wextra 
LDLIBS = -lm -lrt -g

Prog : main.o stack.o idStack.o
	gcc -o Prog main.o stack.o idStack.o $(CFLAGS) $(LDLIBS)

clean:
	rm *.o
