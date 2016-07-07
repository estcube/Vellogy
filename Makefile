CFLAGS = -Wall -pedantic-errors -Wextra 
LDLIBS = -lm -lrt -g --short-enums

Prog : main.o stack.o idStack.o
	gcc -o Prog main.o stack.o idStack.o $(CFLAGS) $(LDLIBS)

stack.o : stack.c
	gcc -c -o stack.o stack.c $(CFLAGS) $(LDLIBS)

idStack.o : idStack.c
	gcc   -c -o idStack.o idStack.c $(CFLAGS) $(LDLIBS)

main.o : main.c
	gcc  -c -o main.o main.c $(CFLAGS) $(LDLIBS)

clean:
	rm *.o
