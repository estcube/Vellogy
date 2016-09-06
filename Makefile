CFLAGS = -Wall -pedantic-errors -Wextra 
LDLIBS = -lm -lrt -g --short-enums

Prog : stack.o idStack.o fftStack.o kiss_fft.o kiss_fftr.o fftFreq.o main.o
	gcc -o Prog stack.o idStack.o fftStack.o kiss_fft.o kiss_fftr.o fftFreq.o main.o $(CFLAGS) $(LDLIBS)

stack.o : stack.c
	gcc -c -o stack.o stack.c $(CFLAGS) $(LDLIBS)

idStack.o : idStack.c
	gcc   -c -o idStack.o idStack.c $(CFLAGS) $(LDLIBS)

fftStack.o : fftStack.c
	gcc   -c -o fftStack.o fftStack.c $(CFLAGS) $(LDLIBS)

main.o : main.c
	gcc  -c -o main.o main.c $(CFLAGS) $(LDLIBS)

clean:
	rm *.o
