#include <stdio.h>
#include <stdlib.h>
#include "stack.h"

int main()

{
    Stack* myStack;
	/*float a = 1.29855685;
	float b = 485162.269648;
	float c = 9462.655;*/
	/*char a = 'a';
	char b = 'A';
	char c = 'z';*/
	int a = 2;
	int b = 85741321;
	int c = 2048;
	/*void* adresse2;*/
	/*int d = -1000000000;
	int e = -2100000000;*/
	myStack = initialize(4);


    	stackPush(myStack, &a);
	stackPush(myStack, &b);
	stackPush(myStack, &c);
	/*stackPush(myStack, &d);
	stackPush(myStack, &e);*/
    /*stackPush(myStack, 8);
    stackPush(myStack, 15);
    stackPush(myStack, 16);
    stackPush(myStack, 23);
    stackPush(myStack, 42);*/

    printf("\nStack state :\n");
    printStack(myStack);

	/*printf("Je depile %d\n", stackPop(myStack));

    printf("Je depile %d\n", stackPop(myStack));


    printf("\nStack state :\n");

    printStack(myStack);*/
/*adresse2 = stackPop(myStack,adresse2);
free(adresse2);
adresse2 = stackPop(myStack,adresse2);
free(adresse2);
adresse2 = stackPop(myStack,adresse2);
free(adresse2);*/
/*printf(" AdresseNB : %d\n", *(int*)numberAdress);*/
/*
printf("Adresse 2 : %d\n", *(int*)adresse2);
stackPop(myStack,adresse2);
printf("Adresse 2 : %d\n", *(int*)adresse2);
stackPop(myStack,adresse2);
printf("Adresse 2 : %d\n", *(int*)adresse2);
printf("\nStack state :\n");*/
printStack(myStack);
/*
stackPop(myStack);
stackPop(myStack);
stackPop(myStack);
stackPop(myStack);
stackPop(myStack);
stackPop(myStack);
stackPop(myStack);*/
deinitialize(myStack);
    return 0;
}
